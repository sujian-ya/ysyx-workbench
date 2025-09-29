#include <isa.h>
#include <debug/etrace.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // 使用 etrace_raise_intr 记录异常触发时的状态
  etrace_raise_intr(top->mcause, top->mepc, top->mtvec, top->mstatus);

  top->mepc = epc;
  top->mcause = NO;

  // mstatus 处理逻辑
  word_t old_mstatus = top->mstatus;
  // 将 MIE（第3位）清零
  top->mstatus &= ~(1 << 3);
  // 将旧的 MIE 移位到 MPIE（第7位）
  top->mstatus |= ((old_mstatus >> 3) & 1) << 7;
  // 将 MPP（第11-12位）设置为机器模式
  top->mstatus |= 3 << 11;

  // 使用 etrace_handle_intr 记录异常处理后的状态
  etrace_handle_intr(top->mcause, top->mepc, top->mtvec, top->mstatus);

  return top->mtvec;
}

word_t isa_ret_intr() {
  // mret 指令，即将返回
  word_t old_mstatus = top->mstatus;

  // 恢复 MIE，从 MPIE（第7位）获取旧值
  top->mstatus |= ((old_mstatus >> 7) & 1) << 3;
  // 清零 MPIE
  top->mstatus &= ~(1 << 7);

  // etrace_trap 用来在陷阱处理结束时打印日志
  etrace_trap(top->mcause, top->mepc, top->mtvec, top->mstatus);

  return top->mepc;
}

// word_t isa_query_intr() {
//   return INTR_EMPTY;
// }
