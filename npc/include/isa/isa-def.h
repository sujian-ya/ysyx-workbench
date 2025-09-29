#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

typedef struct {
  word_t gpr[MUXDEF(CONFIG_RVE, 16, 32)];
  vaddr_t pc;
  word_t inst;
  word_t mepc;
  word_t mstatus;
  word_t mcause;
  word_t mtvec;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

#endif