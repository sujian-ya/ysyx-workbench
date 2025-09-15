#ifndef __DTRACE_H__
#define __DTRACE_H__

#include <common.h>

void log_dtrace(bool is_write, paddr_t addr, int len, word_t data, const char *map_name);

#endif
