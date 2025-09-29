#ifndef __CSR_H__
#define __CSR_H__

#include <common.h>
#include <cpu/cpu.h>

extern "C" word_t csr_read(vaddr_t csr_addr);
extern "C" void csr_write(vaddr_t csr_addr, word_t data);

#endif
