#include <sdb.h>
#include <cpu.h>
#include <utils.h>
#include <memory/paddr.h>
#include <common.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <ysyx_25040105_soc_top.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

extern void cpu_exec(uint64_t n);
extern void sim_exit();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  // 为提示符 (npc) 添加颜色方便操作
  line_read = readline("(\33[1;34mnpc\33[0m) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  npc_state.state = NPC_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char* args) {
	int N;
	if (args == NULL) N = 1;
	else sscanf(args, "%d", &N);
	cpu_exec(N);
  if (npc_state.state == NPC_STOP) {
    #ifndef CONFIG_WATCHPOINT
        printf("pc = %s%08x%s\n", ANSI_FG_GREEN, cpu.pc, ANSI_NONE);
    #endif
  }
	return 0;
}

static int cmd_info(char* args) {
	if (args == NULL) printf("No args.\n");
	if (strcmp(args, "r") == 0) npc_reg_display();
	if (strcmp(args, "w") == 0) sdb_watchpoint_display();
	return 0;
}

static int cmd_d (char* args) {
	if (args == NULL) {
		printf("No args\n");
		return 0;
	}
	char *num_str = strtok(args, " ");
	while (num_str != NULL) {
		int num = atoi(num_str);
		delete_watchpoint(num);
		num_str = strtok(NULL, " ");
	}
	//else delete_watchpoint(atoi(args));
	return 0;
}

static int cmd_w(char* args) {
	if (args == NULL) printf("No args.\n");
	create_watchpoint(args);
	return 0;
}

static int cmd_x(char* args){
	char *num_str = strtok(args, " ");
	int num = atoi(num_str);
	char *expr_str = args + strlen(num_str) + 1;
	bool success = true;
	uint32_t expr_num = (uint32_t)expr(expr_str, &success);
	if (!success) {
    printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK,cpu.pc, ANSI_NONE);
		printf("Calculating error\n");
		return 0;
	}
  printf("%s\n", ANSI_FMT("Scaning memory and displaying corresponding value:", ANSI_FG_LIGHTPINK));
  printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
	printf("%-14s %-14s %-14s\n", "address", "uint32_t", "int32_t");
	for (int i = 0; i < num; i++) {
		uint32_t mem = expr_num + 4 * i;
		paddr_t value = paddr_read(mem, 4);
		printf("| 0x%-10x | 0x%-10x | %-12d\n", mem, (uint32_t)value, (int32_t)value);
	}
	return 0;
}

static int cmd_p(char* args) {
	if (args == NULL) {
		printf("No expression provided.\n");
		return 0;
	}
	bool success = true;
	//word_t result = expr(args, &success);
	int32_t val = (int32_t)expr(args, &success);
	if (!success) {
		// printf("Error evaluating expression.\n");
    return 0;
	}

	printf("%s\n", ANSI_FMT("Calculating expression:", ANSI_FG_LIGHTPINK));
  printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
	printf("%s%-14s %-14s %-14s%s\n", ANSI_FG_BLACK, "uint32_t", "int32_t", "expression", ANSI_NONE);
	printf("| %s0x%-10x | %-12d | %s%s\n", ANSI_FG_BLACK, (uint32_t)val, (int32_t)val, args, ANSI_NONE);
	return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c"   , "Continue the execution of the program", cmd_c },
  { "q"   , "Exit NEMU", cmd_q },
  { "si"  , "Let the program pause execution after running N instructions step by step. When N is not given, it defaults to 1", cmd_si },
	{ "info", "Print register status - r, print watchpoint information - w", cmd_info },
  { "x"   , "Calculate the value of the expression EXPR, and use the result as the starting memory address, output the continous N bytes in hexadecimal format.Usage: x N EXPR", cmd_x },
  { "p"   , "Evaluate the expression EXPR.", cmd_p },
	{ "w"   , "Set watchpoints.When the value of the expression EXPR changes, pause the program execution", cmd_w},
	{ "d"   , "Delete the watchpoint with serial number N", cmd_d},
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s%s%s - %s\n", ANSI_FG_BLACK, cmd_table[i].name, ANSI_NONE, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s%s%s - %s\n", ANSI_FG_BLACK, cmd_table[i].name, ANSI_NONE, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  // 声明NPC状态
  NPCState npc_state = { .state = NPC_STOP };

  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
		/* '+ 1': skip space*/
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

//   /* Initialize the watchpoint pool. */
  init_wp_pool();
}
