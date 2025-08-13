#ifndef __PMEM_H__
#define __PMEM_H__

#include <stdint.h>

#define PMEM_SIZE (128 * 1024 * 1024) // 128 MB

extern uint32_t pmem[PMEM_SIZE / 4]; // 内存空间，改为uint32_t以字为单位访问

// 初始化内存，加载bin文件
void pmem_init(const char* bin_file);

// 内存读函数
uint32_t pmem_read(uint32_t addr);

#endif
