#ifndef __DEBUG_ETRACE_H__
#define __DEBUG_ETRACE_H__

#include <common.h>
#include <cpu/cpu.h>

void etrace_raise_intr(word_t mcause, vaddr_t mepc, vaddr_t mtvec, word_t mstatus);
void etrace_handle_intr(word_t mcause, vaddr_t mepc, vaddr_t mtvec, word_t mstatus);
void etrace_trap(word_t mcause, vaddr_t mepc, vaddr_t mtvec, word_t mstatus);

#endif
