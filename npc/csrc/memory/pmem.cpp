#include <common.h>
#include <pmem.h>
#include <utils.h>
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
  // 计算4字节对齐地址
  int aligned_addr = addr & ~0x3u;
  
  // 校验对齐结果是否正确（必须4字节对齐）
  assert((aligned_addr & 0x3) == 0);
  
  // 计算正确的内存索引（考虑基地址和4字节单位）
  uint32_t index = (aligned_addr - CONFIG_MBASE) >> 2;
  
  // 校验索引是否在有效范围内
  assert(index < CONFIG_MSIZE / 4);
  
  return pmem[index];
}

extern "C" void pmem_write(int waddr, int wdata, char wmask) {
    // 计算4字节对齐地址
    int aligned_addr = waddr & ~0x3u;
    
    // 验证地址是否4字节对齐
    assert((aligned_addr & 0x3) == 0);
    
    // 计算内存索引（考虑基地址和4字节单位）
    uint32_t index = (aligned_addr - CONFIG_MBASE) >> 2;
    assert(index < CONFIG_MSIZE / 4);  // 确保索引在有效范围内
    
    // 获取当前内存中的4字节数据
    uint32_t current_data = pmem[index];
    uint32_t new_data = current_data;
    
    // 根据wmask逐字节更新数据
    // wmask的每一位对应1个字节是否需要更新（1表示更新，0表示保持）
    if (wmask & 0x01) {
        // 更新第0个字节（最低字节）
        new_data = (new_data & ~0x000000FF) | (wdata & 0x000000FF);
    }
    if (wmask & 0x02) {
        // 更新第1个字节
        new_data = (new_data & ~0x0000FF00) | (wdata & 0x0000FF00);
    }
    if (wmask & 0x04) {
        // 更新第2个字节
        new_data = (new_data & ~0x00FF0000) | (wdata & 0x00FF0000);
    }
    if (wmask & 0x08) {
        // 更新第3个字节（最高字节）
        new_data = (new_data & ~0xFF000000) | (wdata & 0xFF000000);
    }
    
    // 将更新后的数据写回内存
    pmem[index] = new_data;
}
