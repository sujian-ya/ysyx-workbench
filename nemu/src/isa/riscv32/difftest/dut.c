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

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  // 比较PC
  if (cpu.pc != ref_r->pc) {
    printf("Mismatch at pc = 0x%08x: DUT pc = 0x%08x, REF pc = 0x%08x\n", pc, cpu.pc, ref_r->pc);
    return false;
  }

  // 比较寄存器
  for (int i = 0; i < 32; i++) {
    if (gpr(i) != ref_r->gpr[i]) {
      printf("Mismatch at pc = 0x%08x: reg x%d, DUT = 0x%08x, REF = 0x%08x\n", pc, i, gpr(i), ref_r->gpr[i]);
      return false;
    }
  }

  return true;
}

void isa_difftest_attach() {
}

#define REG_NUM 32

void diff_reg_display(CPU_state *ref_r) {
	printf("Displaying reg(diff):\n");
	printf("pc = 0x%-10x\n", (uint32_t)ref_r->pc);
	// printf("%-8s %-12s\n", "REG_NAME", "VALUE");
	int cnt = 1;
	for (int i = 0; i < REG_NUM; i++, cnt++) {
		word_t val = ref_r->gpr[i];
		if (cnt % 4 == 0) {
			printf("|%-8s 0x%-8.8x\n", regs[i], (uint32_t)val);
		} else {
			printf("|%-8s 0x%-8.8x  ", regs[i], (uint32_t)val);
		}
	}
	printf("|%-8s 0x%-8.8x  ", "mepc", (uint32_t)ref_r->mepc);
	printf("|%-8s 0x%-8.8x  ", "mstatus", (uint32_t)ref_r->mstatus);
	printf("|%-8s 0x%-8.8x  ", "mcause", (uint32_t)ref_r->mcause);
	printf("|%-8s 0x%-8.8x\n", "mtvec", (uint32_t)ref_r->mtvec);
}
