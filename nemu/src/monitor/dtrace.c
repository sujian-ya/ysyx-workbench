#include <dtrace.h>
#include <common.h>
#include <utils.h>

void log_dtrace(bool is_write, paddr_t addr, int len, word_t data, const char *map_name){
#ifdef CONFIG_DTRACE
    if(is_write) {
        log_write("[DTRACE] WRITE: addr = 0x%08x, len = %d, data = 0x%08x, device = %s\n",
            addr, len, data, map_name);
    } else {
        log_write("[DTRACE] READ: addr = 0x%08x, len = %d, data = 0x%08x, device = %s\n",
                addr, len, data, map_name);
    }
#endif
}
