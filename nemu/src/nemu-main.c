/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/
#include <sdb.h>
#include <common.h>

#define LINE_LEN 65536
#define EXPR_LEN 65536
void read_file() {
	int cnt = 1;
	FILE *file = fopen("/home/sheep/ysyx-workbench/nemu/tools/gen-expr/input", "r");
	if (file == NULL) {
		perror("Can't open the file \"input\"");
		return;
	}

	char line[LINE_LEN];
	while (fgets(line, sizeof(line), file) != NULL) {
		uint32_t gen_num = 0;
		uint32_t expr_num = 0;
		char expr_str[EXPR_LEN];
		if (sscanf(line, "%u %65536[^\n]", &gen_num, expr_str) == 2) {
			bool success = true;
			expr_num =  (uint32_t)expr(expr_str, &success);
			if (gen_num != expr_num) {
				printf("%04d:  gen-expr: 0x%-10x expr: 0x%-10x expr_str: %s\n",
						cnt++, gen_num, expr_num, expr_str);
			}
			else {
				printf("%04d: no diffenence\n", cnt++);
			}
		}
	}

	fclose(file);
}

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

	/* Compare expression value */
	//read_file();

  /* Start engine. */
	engine_start();

  return is_exit_status_bad();
}
