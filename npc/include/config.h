#ifndef __CONFIG_H__
#define __CONFIG_H__

// isa
#define __GUEST_ISA__ riscv32
#define CONFIG_ISA_riscv 1

// trace
#define CONFIG_TRACE 1
#if defined(CONFIG_TRACE)
// itrace
// #define CONFIG_ITRACE 1

// mtrace
// #define CONFIG_MTRACE 1
// #define CONFIG_MTRACE_START 0x80000000
// #define CONFIG_MTRACE_END   0x8fffffff

// ftrace
// #define CONFIG_FTRACE 1

// dtrace
#define CONFIG_DTRACE 1

// sdb -> watchpoint
// #define CONFIG_WATCHPOINT 1
#endif

// difftest
#define CONFIG_DIFFTEST 1

// memory
#define CONFIG_PC_RESET_OFFSET 0x0
#define CONFIG_PMEM_MALLOC 1
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000
#define CONFIG_MEM_RANDOM 1
#define PAGE_SHIFT        12
#define PAGE_SIZE         (1ul << PAGE_SHIFT)
#define PAGE_MASK         (PAGE_SIZE - 1)

// timer
#define CONFIG_TIMER_GETTIMEOFDAY  1

// reg
#define REG_NUM 32

// device
#define CONFIG_DEVICE 1
#define CONFIG_SERIAL_MMIO 0xa00003f8
#define CONFIG_HAS_SERIAL 1

#endif
