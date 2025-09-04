#include <common.h>
#include <pmem.h>
#include <mtrace.h>
#include <ysyx_25040105_soc_top.h>

#if   defined(CONFIG_PMEM_MALLOC)
// static uint32_t *pmem = NULL;
uint32_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = (uint32_t *)malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

uint32_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }

extern "C" int pmem_read(int addr) {
  // printf("pmem_read get addr: %08x\n", addr);
  int aligned_addr = addr & ~0x3u;
  assert((aligned_addr & 0x3) == 0);
  uint32_t index = (aligned_addr - CONFIG_MBASE) >> 2;
  assert(index < CONFIG_MSIZE / 4);
  log_mtrace(false, aligned_addr, 4, pmem[index]);
  return pmem[index];
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  int aligned_addr = waddr & ~0x3u;
  assert((aligned_addr & 0x3) == 0);
  uint32_t index = (aligned_addr - CONFIG_MBASE) >> 2;
  assert(index < CONFIG_MSIZE / 4);  // 确保索引在有效范围内
  uint32_t current_data = pmem[index];
  uint32_t new_data = current_data;
  if (wmask & 0x01) {
      new_data = (new_data & ~0x000000FF) | (wdata & 0x000000FF);
  }
  if (wmask & 0x02) {
      new_data = (new_data & ~0x0000FF00) | (wdata & 0x0000FF00);
  }
  if (wmask & 0x04) {
      new_data = (new_data & ~0x00FF0000) | (wdata & 0x00FF0000);
  }
  if (wmask & 0x08) {
      new_data = (new_data & ~0xFF000000) | (wdata & 0xFF000000);
  }
  log_mtrace(true, aligned_addr, 4, new_data);
  pmem[index] = new_data;
}
