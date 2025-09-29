#include <csr.h>
#include <isa.h>
#include <debug.h>

extern "C" word_t csr_read(vaddr_t csr_addr) {
  switch(csr_addr) {
    case 0x300: return top->mstatus; // Machine Status Register (treated as 32-bit data)
    case 0x305: return top->mtvec;   // Machine Trap-Vector Base Address
    case 0x341: return top->mepc;    // Machine Exception Program Counter
    case 0x342: return top->mcause;  // Machine Cause Register
    default: panic("csr_read: unimplemented CSR address 0x%08x", csr_addr);
  }
}

extern "C" void csr_write(vaddr_t csr_addr, word_t data) {
  // RISC-V M 模式 mstatus 寄存器可写位的掩码（RV32M 简化版，主要包括 U/S/MIE/MPIE/MPP）
  // MPP (11:12), MPIE (7), MIE (3), SPIE (5), SIE (1), SPP (8)
  // 如果需要，请根据您的实现精确调整
  const word_t MSTATUS_WMASK = 
      (0b11 << 11) | (1 << 7) | (1 << 3) | 
      (1 << 5) | (1 << 1) | (1 << 8); 
  
  switch(csr_addr) {
    case 0x300: 
        // 读出旧值，只更新 MSTATUS_WMASK 对应的位
        // 保留不可写位和保留位
        top->mstatus = (top->mstatus & (~MSTATUS_WMASK)) | (data & MSTATUS_WMASK); 
        break;
    case 0x305: top->mtvec = data;   break;
    case 0x341: top->mepc = data;    break;
    case 0x342: top->mcause = data;  break;
    default: panic("csr_write: unimplemented CSR address 0x%08x", csr_addr);
  }
}
