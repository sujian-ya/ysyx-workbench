#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

void sdb_mainloop();
void init_regex();
// void init_wp_pool();

uint32_t expr(char *e, bool *success);

// 初始化sdb
void init_sdb();

#endif
