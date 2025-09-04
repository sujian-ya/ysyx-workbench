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

static inline bool in_pmem(paddr_t addr) {
  return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

extern "C" int pmem_read(int addr) {
  word_t aligned_addr = addr & ~0x3u;
  if (likely(in_pmem(aligned_addr))) {
    paddr_t index = (aligned_addr - CONFIG_MBASE) >> 2;
    log_mtrace(false, aligned_addr, 4, pmem[index]);
    return pmem[index];
  }
  out_of_bound(aligned_addr);
  return 0;
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  word_t aligned_addr = waddr & ~0x3u;
  if (likely(in_pmem(aligned_addr))) {
    paddr_t index = (aligned_addr - CONFIG_MBASE) >> 2;
    word_t current_data = pmem[index];
    word_t new_data = current_data;
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
    pmem[index] = new_data;
    log_mtrace(true, aligned_addr, 4, new_data);
    return;
  }
  out_of_bound(aligned_addr);
}
