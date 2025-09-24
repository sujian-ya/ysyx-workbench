#include <isa.h>
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

extern "C" word_t vaddr_read(int addr, int len) {
  return paddr_read((vaddr_t)addr, len);
}

extern "C" void vaddr_write(int addr, int len, int data) {
  paddr_write(addr, len, data);
}
