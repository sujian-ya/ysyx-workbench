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
	if (args == NULL || *args == '\0') {
		printf("Usage: w EXPR\n");
		return ;
	}
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

	printf("Watchpoint: %02d\texpr: %-16s value: %d (0x%x)\n",
			new->NO, new->expr, (int32_t)new->value, new->value);
}

void delete_watchpoint(int no) {
	WP *current = head;

	while (current != NULL) {
		if (current->NO == no) {
			free_wp(current);
			printf("Deleted watchpoint %02d\n", no);
			return;
		}
		current = current->next;
	}

	printf("Watchpoint %02d not found.\n", no);
}

void sdb_watchpoint_display() {
	if (head == NULL) {
		printf("No watchpoint.\n");
		return;
	}

	WP *current = head;
	while (current != NULL) {
		printf("Watchpoint: %02d\texpr: %-16s value: %d (0x%x)\n",
			 	current->NO, current->expr, (int32_t)current->value, current->value);
		current = current->next;
	}
}

#ifdef CONFIG_WATCHPOINT
void check_watchpoint() {
	printf("Checking watchpoint...\n");
	if (nemu_state.state == NEMU_END) {
		return;
	}
	for (WP *wp = head; wp; wp = wp->next) {
		bool success = true;
		word_t cur_value = expr(wp->expr, &success);

		if (!success) {
			printf("Watchpoint %02d expression invalid: %s\n", wp->NO, wp->expr);
			continue;
		}

		if (cur_value != wp->value) {
			nemu_state.state = NEMU_STOP;
			printf("Watchpoint: %02d\texpr: %-16s Old value: 0x%x\tNew value: 0x%x\n",
					wp->NO, wp->expr, wp->value, cur_value);
			wp->value = cur_value;
		} else {
			printf("Watchpoint: %02d (No change)\n", wp->NO);
			nemu_state.state = NEMU_RUNNING;
		}
	}
	return;
}
#endif
