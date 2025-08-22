#ifndef __CONFIG_H__
#define __CONFIG_H__

// timer.cpp
// 开启时间计算
#define CONFIG_TIMER_GETTIMEOFDAY  1

// init.cpp
// 客服端isa选择
#define __GUEST_ISA__ riscv32e

// reg.cpp
#define REG_NUM 32

// watchpoint.cpp
// #define CONFIG_WATCHPOINT 1

// disasm.cpp
#define CONFIG_ITRACE 1
#define CONFIG_ISA_riscv 1

// ftrace.cpp
#define CONFIG_FTRACE 1

#endif
