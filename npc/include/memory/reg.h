#ifndef __MEMORY_REG_H__
#define __MEMORY_REG_H__

#include <common.h>

// 打印寄存器
extern void npc_reg_display();

// 获取寄存器的值
word_t npc_reg_str2val(const char *s, bool *success);

extern const char* regs[];

#endif