#include <pmem.h>
#include <common.h>
#include <utils.h>
#include <config.h>
#include <ysyx_25040105_soc_top.h>

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

// 打印 NPC 的寄存器
void npc_reg_display() {
    printf("%s\n", ANSI_FMT("Displaying reg:", ANSI_FG_GREEN));
    printf("pc = %s%08x%s\n", ANSI_FG_BLACK, pc, ANSI_NONE);
    printf("%s%-14s %-14s %-14s%s\n", ANSI_FG_BLACK, "reg_name", "uint32_t", "int32_t", ANSI_NONE);
    for (int i = 0; i < REG_NUM; i++) {
        printf("| %s%-12s | 0x%-10x | %-12d%s\n", ANSI_FG_BLACK, regs[i], reg[i], (int32_t)reg[i], ANSI_NONE);
    }
}

word_t npc_reg_str2val(const char *s, bool *success) {
	for (int i = 0; i < REG_NUM; i++) {
		if (strcmp(s, regs[i]) == 0) {
			// word_t val = gpr(check_reg_idx(i));
      word_t val = (word_t)reg[i];
			*success = true;
			return val;
		}
	}
	*success = false;
  return 0;
}