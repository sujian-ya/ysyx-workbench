#ifndef __PMEM_H__
#define __PMEM_H__

#include <stdint.h>
#include <common.h>

#define CONFIG_PMEM_MALLOC 1
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000
#define CONFIG_MEM_RANDOM 1
#define PMEM_LEFT  ((paddr_t)CONFIG_MBASE)
#define PMEM_RIGHT ((paddr_t)CONFIG_MBASE + CONFIG_MSIZE - 1)
#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)

extern uint32_t *pmem;
// 初始化内存，加载bin文件
void init_mem();

// 内存读函数
extern "C" int pmem_read(int addr);

// 打印寄存器
extern void npc_reg_display();

// 获取寄存器的值
word_t npc_reg_str2val(const char *s, bool *success);

uint32_t* guest_to_host(paddr_t paddr);

#endif
