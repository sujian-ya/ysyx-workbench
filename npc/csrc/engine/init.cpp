#include <isa.h>
#include <init.h>
#include <common.h>
#include <memory/paddr.h>
#include <ysyx_25040105_soc_top.h>

void sim_init();

void init_npc(int argc, char *argv[]) {
  sim_init();
  contextp->commandArgs(argc, argv);
  reset(5);
  // for (int i = 0; i < argc; i++) {
  //   Log("NPC-init argc[%d]: %s", i, argv[i]);
  // }
}

static const uint32_t img [] = {
  0x00000297,  // auipc t0,0
  0x00028823,  // sb  zero,16(t0)
  0x0102c503,  // lbu a0,16(t0)
  0x00100073,  // ebreak (used as nemu_trap)
  0xdeadbeef,  // some data
};

static void restart() {
  /* Set the initial program counter. */
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  cpu.gpr[0] = 0;
}

void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  restart();
}
