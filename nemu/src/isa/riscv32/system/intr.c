/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <etrace.h>

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // 使用 etrace_raise_intr 记录异常触发时的状态
  etrace_raise_intr(cpu.mcause, cpu.mepc, cpu.mtvec, cpu.mstatus);

  cpu.mepc = epc;
  cpu.mcause = NO;

  // mstatus 处理逻辑
  word_t old_mstatus = cpu.mstatus;
  // 将 MIE（第3位）清零
  cpu.mstatus &= ~(1 << 3);
  // 将旧的 MIE 移位到 MPIE（第7位）
  cpu.mstatus |= ((old_mstatus >> 3) & 1) << 7;
  // 将 MPP（第11-12位）设置为机器模式
  cpu.mstatus |= 3 << 11;

  // 使用 etrace_handle_intr 记录异常处理后的状态
  etrace_handle_intr(cpu.mcause, cpu.mepc, cpu.mtvec, cpu.mstatus);

  return cpu.mtvec;
}

word_t isa_ret_intr() {
  // mret 指令，即将返回
  word_t old_mstatus = cpu.mstatus;

  // 恢复 MIE，从 MPIE（第7位）获取旧值
  cpu.mstatus |= ((old_mstatus >> 7) & 1) << 3;
  // 清零 MPIE
  cpu.mstatus &= ~(1 << 7);

  // etrace_trap 用来在陷阱处理结束时打印日志
  etrace_trap(cpu.mcause, cpu.mepc, cpu.mtvec, cpu.mstatus);

  return cpu.mepc;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
