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

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.mepc = epc;
  cpu.mcause = NO;
  // mstatus 处理逻辑
  word_t old_mstatus = cpu.mstatus;
  cpu.mstatus &= ~(1 << 3);
  cpu.mstatus |= ((old_mstatus >> 3) & 1) << 7;
  cpu.mstatus |= 3 << 11;
  return cpu.mtvec;
}

word_t isa_ret_intr() {
  word_t old_mstatus = cpu.mstatus;
  // 恢复MIE
  cpu.mstatus |= ((old_mstatus >> 7) & 1) << 3;
  // 清零MPIE
  cpu.mstatus &= ~(1 << 7);
  return cpu.mepc;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
