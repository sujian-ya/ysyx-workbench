#ifndef __DEBUG_FTRACE_H__
#define __DEBUG_FTRACE_H__

#include <common.h>

// 加载和初始化elf文件
long load_elf(const char *elf_file);
void init_elf(const char *elf_file);

// ftrace相关函数声明
// 这两个函数在指令执行时被调用，记录函数调用和返回信息
void ftrace_call(vaddr_t pc, vaddr_t target);
void ftrace_ret(vaddr_t pc, vaddr_t ret_addr);

#endif
