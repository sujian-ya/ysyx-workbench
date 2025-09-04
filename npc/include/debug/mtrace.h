#ifndef __DEBUG_MTRACE_H__
#define __DEBUG_MTRACE_H__

#include <common.h>
#include <config.h>

void log_mtrace(bool is_write, paddr_t addr, int len, word_t data);

#endif
