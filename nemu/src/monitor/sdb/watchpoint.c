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

#include <cpu/decode.h>
#include "sdb.h"
#include <common.h>

#define NR_WP 32
#define EXPR_LEN 256

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
	char expr[EXPR_LEN];
	word_t value;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i+1;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
	if (free_ == NULL) {
		printf("No free watchpoints available.\n");
		assert(0); // No available idle monitoring points.
		return NULL;
	}

	// Extract a free node from the free_ list.
	WP *new = free_;
	free_ = free_->next;

	// Insert a new node at the beginning of the head linked list.
	new->next = head;
	head = new;

	// Set the unique identifier for the monitoring point.
	return new;
}

void free_wp(WP *wp) {
	// Remove wp from the head linked list.
	if (head == NULL) return;

	WP *prev = NULL;
	WP *current = head;
	while (current != NULL && current != wp) {
		prev = current;
		current = current->next;
	}

	if (current == NULL) {
		return; // wp is not in the head list.
	}

	if (prev == NULL) {
		head = wp->next; // Remove head node.
	} else {
		prev->next = wp->next;
	}

	// Insert wp into the head of the free_ linked list.
	wp->next = free_;
	free_ = wp;
}

void create_watchpoint(char* args) {
	WP *new = new_wp();
	if (new == NULL) return;

	strncpy(new->expr, args, EXPR_LEN - 1);
	new->expr[EXPR_LEN - 1] = '\0';

	bool watchpoint_success = true;
	new->value = expr(new->expr, &watchpoint_success);
	if (!watchpoint_success) {
		printf("Invalid expression '%s'.\n", args);
		free_wp(new);
		return;
	}

	printf("Creating watchpoint:\n");
	printf("%-12s %-12s %-12s %-12s %-12s\n","PC","Watchpoint","int32_t","uint32_t","expression");
	printf("0x%-10x %-12d %-12d 0x%-10x %s\n",
			cpu.pc, new->NO, (int32_t)new->value, (uint32_t)new->value, new->expr);
}

void delete_watchpoint(int no) {
	WP *current = head;

	while (current != NULL) {
		if (current->NO == no) {
			free_wp(current);
			printf("0x%-10x %-12d %-12d 0x%-10x %s\n",
					cpu.pc, current->NO, (int32_t)current->value, (uint32_t)current->value, current->expr);
			return;
		}
		current = current->next;
	}

	printf("0x%-10x %-12d (no found watchpoint)\n", cpu.pc, no);
}

void sdb_watchpoint_display() {
	if (head == NULL) {
		printf("No watchpoint.\n");
		return;
	}

	WP *current = head;
	printf("Displaying watchpoint:\n");
	printf("%-12s %-12s %-12s %-12s %-12s\n","PC","Watchpoint","int32_t","uint32_t","expression");
	while (current != NULL) {
		printf("0x%-10x %-12d %-12d 0x%-10x %s\n",
			 	cpu.pc, current->NO, (int32_t)current->value, (uint32_t)current->value, current->expr);
		current = current->next;
	}
}

#ifdef CONFIG_WATCHPOINT
void check_watchpoint(void) {
	if (nemu_state.state == NEMU_END) {
		return;
	}
	int cnt = 0;
	printf("\nChecking watchpoint:\n");
	printf("%-12s %-12s %-12s %-12s\n","PC","Watchpoint","Old_value","New_value");
	for (WP *wp = head; wp; wp = wp->next, cnt++) {
		bool success = true;
		word_t cur_value = expr(wp->expr, &success);

		if (!success) {
			printf("Watchpoint %02d expression invalid: %s\n", wp->NO, wp->expr);
			continue;
		}

		if (cur_value != wp->value) {
			nemu_state.state = NEMU_STOP;
			printf("0x%-10x %-12d 0x%-10x 0x%-10x\n",
					cpu.pc, wp->NO, (uint32_t)wp->value, (uint32_t)cur_value);
			wp->value = cur_value;
		} else {
			printf("0x%-10x %-12d (no change)\n", cpu.pc, wp->NO);
			nemu_state.state = NEMU_RUNNING;
		}
	}
	if (!cnt) {
		printf("0x%-10x (no watchpoint)\n", cpu.pc);
	}
	return;
}
#endif
