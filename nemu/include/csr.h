#ifndef __CSR_H__
#define __CSR_H__

#include <common.h>
#include <cpu/cpu.h>

word_t csr_read(int csr_addr);
void csr_write(int csr_addr, word_t data);

#endif
