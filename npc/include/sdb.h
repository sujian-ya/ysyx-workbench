#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

void sdb_mainloop();
void init_regex();
void init_wp_pool();

uint32_t expr(char *e, bool *success);
void create_watchpoint(char* args);
void delete_watchpoint(int no);
void sdb_watchpoint_display();

// 初始化sdb
void init_sdb();

#ifdef CONFIG_WATCHPOINT
void check_watchpoint();
#endif

#endif
