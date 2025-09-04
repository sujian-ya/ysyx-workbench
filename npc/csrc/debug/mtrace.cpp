#include <common.h>
#include <mtrace.h>

// mtrace函数 
void log_mtrace(bool is_write, paddr_t addr, int len, word_t data) {
#ifdef CONFIG_MTRACE
  if (addr >= CONFIG_MTRACE_START && addr <= CONFIG_MTRACE_END) {
    char buf[128];
    if (is_write) {
      switch(len) {
        //FMT_PADDR是一个宏，用于格式化物理地址
        case 1:   snprintf(buf, sizeof(buf), ANSI_FMT("write @ ", ANSI_FG_APRICOT) FMT_PADDR ": 0x%02x", addr, (uint32_t)data & 0xFF);     break;
        case 2:   snprintf(buf, sizeof(buf), ANSI_FMT("write @ ", ANSI_FG_APRICOT) FMT_PADDR ": 0x%04x", addr, (uint32_t)data & 0xFFFF);   break;
        case 4:   snprintf(buf, sizeof(buf), ANSI_FMT("write @ ", ANSI_FG_APRICOT) FMT_PADDR ": 0x%08x", addr, (uint32_t)data);            break;
        case 8:   snprintf(buf, sizeof(buf), ANSI_FMT("write @ ", ANSI_FG_APRICOT) FMT_PADDR ": 0x%016" PRIx64, addr, (uint64_t)data);     break;
        default:  snprintf(buf, sizeof(buf), ANSI_FMT("write @ ", ANSI_FG_APRICOT) FMT_PADDR ": unknown len %d", addr, len);               break;
      }
    } else {
      switch(len) {
        case 1:   snprintf(buf, sizeof(buf), ANSI_FMT("read  @ ", ANSI_FG_LAVENDER) FMT_PADDR ": = 0x%02x", addr, (uint32_t)data & 0xFF);   break;
        case 2:   snprintf(buf, sizeof(buf), ANSI_FMT("read  @ ", ANSI_FG_LAVENDER) FMT_PADDR ": = 0x%04x", addr, (uint32_t)data & 0xFFFF); break;
        case 4:   snprintf(buf, sizeof(buf), ANSI_FMT("read  @ ", ANSI_FG_LAVENDER) FMT_PADDR ": = 0x%08x", addr, (uint32_t)data);          break;
        case 8:   snprintf(buf, sizeof(buf), ANSI_FMT("read  @ ", ANSI_FG_LAVENDER) FMT_PADDR ": = 0x%016" PRIx64, addr, (uint64_t)data);   break;
        default:  snprintf(buf, sizeof(buf), ANSI_FMT("read  @ ", ANSI_FG_LAVENDER) FMT_PADDR ": unknown len %d", addr, len);               break;
      }
    }
    _Log("%s\n", buf);
  }
#endif
}
