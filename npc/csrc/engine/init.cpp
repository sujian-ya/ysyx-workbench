#include <init.h>
#include <utils.h>
#include <config.h>
#include <common.h>
#include <ysyx_25040105_soc_top.h>

void sim_init();

void init_npc(int argc, char *argv[]) {
  sim_init();
  contextp->commandArgs(argc, argv);
  reset(5);
  for (int i = 0; i < argc; i++) {
    Log("NPC-init argc[%d]: %s", i, argv[i]);
  }
}
