#include <debug/etrace.h>
#include <common.h>

void etrace_raise_intr(word_t mcause, vaddr_t mepc, vaddr_t mtvec, word_t mstatus) {
#ifdef CONFIG_ETRACE
    // 记录异常触发时的状态
    log_write("[ETRACE] Exception raised  @ | mcause = 0x%08x | mepc = 0x%08x | mtvec = 0x%08x | mstatus = 0x%08x\n", mcause, mepc, mtvec, mstatus);
#endif
}

void etrace_handle_intr(word_t mcause, vaddr_t mepc, vaddr_t mtvec, word_t mstatus) {
#ifdef CONFIG_ETRACE
    // 记录异常处理后的状态
    log_write("[ETRACE] Exception handled @ | mcause = 0x%08x | mepc = 0x%08x | mtvec = 0x%08x | mstatus = 0x%08x\n", mcause, mepc, mtvec, mstatus);
#endif
}

void etrace_trap(word_t mcause, vaddr_t mepc, vaddr_t mtvec, word_t mstatus) {
#ifdef CONFIG_ETRACE
    // 记录陷阱处理结束时的状态
    log_write("[ETRACE] Trap @ | mcause = 0x%08x | mepc = 0x%08x | mtvec = 0x%08x | mstatus = 0x%08x\n", mcause, mepc, mtvec, mstatus);
#endif
}
