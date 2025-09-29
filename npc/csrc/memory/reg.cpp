#include <reg.h>
#include <common.h>
#include <utils.h>
#include <config.h>
#include <ysyx_25040105_soc_top.h>

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
  "mepc", "mstatus", "mcause", "mtvec"
};

// 打印 NPC 的寄存器
void npc_reg_display() {
	printf("Displaying reg(npc):\n");
	printf("pc = 0x%-10x\n", (uint32_t)cpu.pc);
	int cnt = 1;
	for (int i = 0; i < REG_NUM; i++, cnt++) {
		word_t val = cpu.gpr[i];
		if (cnt % 4 == 0) {
			printf("|%-8s 0x%-8.8x\n", regs[i], (uint32_t)val);
		} else {
			printf("|%-8s 0x%-8.8x  ", regs[i], (uint32_t)val);
		}
	}
	printf("|%-8s 0x%-8.8x  ", "mepc", (uint32_t)cpu.mepc);
	printf("|%-8s 0x%-8.8x  ", "mstatus", (uint32_t)cpu.mstatus);
	printf("|%-8s 0x%-8.8x  ", "mcause", (uint32_t)cpu.mcause);
	printf("|%-8s 0x%-8.8x\n", "mtvec", (uint32_t)cpu.mtvec);
}

word_t npc_reg_str2val(const char *s, bool *success) {
	for (int i = 0; i < REG_NUM; i++) {
		if (strcmp(s, regs[i]) == 0) {
			// word_t val = gpr(check_reg_idx(i));
      word_t val = (word_t)cpu.gpr[i];
			*success = true;
			return val;
		}
	}
	*success = false;
  return 0;
}