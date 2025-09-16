/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cpu/decode.h>
#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sdb.h>
#include <memory/paddr.h> /*include paddr_read function*/

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

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
	nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char* args) {
	int N;
	if (args == NULL) N = 1;
	else sscanf(args, "%d", &N);
	cpu_exec(N);
	return 0;
}

static int cmd_info(char* args) {
	if (args == NULL) printf("No args.\n");
	if (strcmp(args, "r") == 0) isa_reg_display();
	if (strcmp(args, "w") == 0) sdb_watchpoint_display();
	return 0;
}

static int cmd_d (char* args) {
	if (args == NULL) {
		printf("No args\n");
		return 0;
	}
	char *num_str = strtok(args, " ");
	printf("Deleting watchpoint:\n");
	printf("%-12s %-12s %-12s %-12s %-12s\n","PC","Watchpoint","int32_t","uint32_t","expression");
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
		printf("Calculating error\n");
		return 0;
	}
	printf("Scaning memory and displaying corresponding value:\n");
	printf("%-12s %-12s %-12s %-12s\n", "PC", "Address", "int32_t", "uint32_t");
	for (int i = 0; i < num; i++) {
		uint32_t mem = expr_num + 4 * i;
		paddr_t value = paddr_read(mem, 4);
		printf("0x%-10x 0x%-10x %-12d 0x%-10x\n", cpu.pc, mem, (uint32_t)value, (int32_t)value);
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
		printf("Error evaluating expression.\n");
	}

	printf("Calculating expression:\n");
	printf("%-12s %-12s %-12s %-20s\n", "PC", "int32_t", "uint32_t", "expression");
	printf("0x%-10x %-12d 0x%-10x %s\n", cpu.pc, (int32_t)val, (uint32_t)val, args);
	return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
	{ "si", "Let the proaram pause exection after running N instructions step by step.When N is not given, it defaults to 1", cmd_si },
	{ "info", "Print register status - r, print watchpoint information - w", cmd_info },
	{ "x", "Calculate the value of the expression EXPR, and use the result as the starting memory address, output the continous N bytes in hexadecimal format.Usage: x N EXPR", cmd_x },
	{ "p", "Evaluate the expression EXPR.", cmd_p },
	{ "w", "Set watchpoints.When the value of the expression EXPR changes, pause the program execution", cmd_w},
	{ "d", "Delete the watchpoint with serial number N", cmd_d},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
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

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

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

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
