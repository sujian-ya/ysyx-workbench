#ifndef __PMEM_H__
#define __PMEM_H__

#include <stdint.h>
#include <common.h>

extern uint32_t *pmem;
// 初始化内存，加载bin文件
void init_mem();

// 内存读函数
uint32_t pmem_read(uint32_t addr);

// 打印寄存器
extern void npc_reg_display();

// 获取寄存器的值
word_t npc_reg_str2val(const char *s, bool *success);

#endif
