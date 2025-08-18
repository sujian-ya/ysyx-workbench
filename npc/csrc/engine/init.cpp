#include <init.h>
#include <utils.h>
#include <config.h>
#include <common.h>
#include <ysyx_25040105_soc_top.h>

void init_npc(int argc, char *argv[]) {
  const char* bin_file = NULL;
  if (argc > 1) {
    bin_file = argv[1];
  }
  sim_init(bin_file);
  contextp->commandArgs(argc, argv);
  reset(5);
}
