#include <csr.h>
#include <isa.h>

word_t csr_read(int csr_addr) {
  switch(csr_addr) {
    case 0x300: return cpu.mstatus; // Machine Status Register (treated as 32-bit data)
    case 0x305: return cpu.mtvec;   // Machine Trap-Vector Base Address
    case 0x341: return cpu.mepc;    // Machine Exception Program Counter
    case 0x342: return cpu.mcause;  // Machine Cause Register
    default: panic("csr_read: unimplemented CSR address 0x%x", csr_addr);
  }
}

void csr_write(int csr_addr, word_t data) {
  switch(csr_addr) {
    case 0x300: cpu.mstatus = data; break; // Machine Status Register (treated as 32-bit data)
    case 0x305: cpu.mtvec = data;   break; // Machine Trap-Vector Base Address
    case 0x341: cpu.mepc = data;    break; // Machine Exception Program Counter
    case 0x342: cpu.mcause = data;  break; // Machine Cause Register
    default: panic("csr_write: unimplemented CSR address 0x%x", csr_addr);
  }
}
