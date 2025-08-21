#include <common.h>
#include <pmem.h>
#include <utils.h> // 使用Log和Assert宏

#define CONFIG_PMEM_MALLOC 1
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000
#define CONFIG_MEM_RANDOM 1
#define PMEM_LEFT  ((paddr_t)CONFIG_MBASE)
#define PMEM_RIGHT ((paddr_t)CONFIG_MBASE + CONFIG_MSIZE - 1)

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

uint32_t pmem_read(uint32_t addr) {
    assert((addr & 0x3) == 0); // 地址必须4字节对齐，直接位数比较加快处理速度
    uint32_t index = (addr - CONFIG_MBASE) >> 2;
    // assert(index < PMEM_SIZE); // 确保地址在范围内
    assert(index < CONFIG_MSIZE / 4); // 确保地址在范围内
    return pmem[index];
}
