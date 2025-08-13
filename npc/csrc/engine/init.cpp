#include <init.h>
#include <utils.h>
#include <config.h>
#include <common.h>
#include <ysyx_25040105_soc_top.h>

extern unsigned char npc_logo[];

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NPC!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("%s%s%s\n", ANSI_FG_WHITE, npc_logo, ANSI_NONE);
  printf("For help, type \"help\"\n");
  //Log("Exercise: Please remove me in the source code and compile NPC again.");
  //assert(0);
}

void init_npc(int argc, char *argv[]) {
  const char* bin_file = NULL;
  if (argc > 1) {
    bin_file = argv[1];
  }
  sim_init(bin_file);
  contextp->commandArgs(argc, argv);
  reset(5);
  welcome();
}
