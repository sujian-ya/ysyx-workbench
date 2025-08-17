#include <sdb.h>
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
		printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, pc, ANSI_NONE);
		printf("No free watchpoints available.\n");
		assert(0); // No available idle monitoring points.
		return NULL;
	}

	// Extract a free node from the free_ list.
	WP *new_node = free_;
	free_ = free_->next;

	// Insert a new node at the beginning of the head linked list.
	new_node->next = head;
	head = new_node;

	// Set the unique identifier for the monitoring point.
	return new_node;
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
	WP *new_node = new_wp();
	if (new_node == NULL) return;

	strncpy(new_node->expr, args, EXPR_LEN - 1);
	new_node->expr[EXPR_LEN - 1] = '\0';

	bool watchpoint_success = true;
	new_node->value = expr(new_node->expr, &watchpoint_success);
	if (!watchpoint_success) {
		printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, pc, ANSI_NONE);
		printf("Invalid expression '%s'.\n", args);
		free_wp(new_node);
		return;
	}

	printf("%s\n", ANSI_FMT("Creating watchpoint:", ANSI_FG_LIGHTPINK));
	printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, pc, ANSI_NONE);
	printf("%-14s %-14s %-14s %-14s\n", "watchpoint", "uint32_t", "int32_t", "expression");
	printf("| %-12d | 0x%-10x | %-12d | %s\n",
			new_node->NO, (uint32_t)new_node->value, (int32_t)new_node->value, new_node->expr);
}

void delete_watchpoint(int no) {
	WP *current = head;

	printf("%s\n", ANSI_FMT("Deleting watchpoint:", ANSI_FG_LIGHTPINK));
	printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, pc, ANSI_NONE);
	printf("%-14s %-14s %-14s %-14s\n", "watchpoint", "uint32_t", "int32_t", "expression");
	while (current != NULL) {
		if (current->NO == no) {
			free_wp(current);
			printf("| %-12d | 0x%-10x | %-12d | %s\n",
					current->NO, (uint32_t)current->value, (int32_t)current->value, current->expr);
			return;
		}
		current = current->next;
	}
	printf("| %-12d | (unfounded watchpoint)\n", no);
}

void sdb_watchpoint_display() {
	if (head == NULL) {
		printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, pc, ANSI_NONE);
		printf("No watchpoint.\n");
		return;
	}

	WP *current = head;
	printf("%s\n", ANSI_FMT("Displaying watchpoint:", ANSI_FG_LIGHTPINK));
	printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, pc, ANSI_NONE);
	printf("%-14s %-14s %-14s %-14s\n", "watchpoint", "uint32_t", "int32_t", "expression");
	while (current != NULL) {
		printf("| %-12d | 0x%-10x | %-12d | %s\n",
			 	current->NO, (uint32_t)current->value, (int32_t)current->value, current->expr);
		current = current->next;
	}
}

#ifdef CONFIG_WATCHPOINT
void check_watchpoint(void) {
	if (npc_state.state == NPC_END) {
		return;
	}
	int cnt = 0;
	printf("%s\n", ANSI_FMT("Checking watchpoint:", ANSI_FG_LIGHTPINK));
	printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, pc, ANSI_NONE);
	printf("%-14s %-14s %-14s %-14s\n", "watchpoint", "old_value", "new_value", "expression");
	for (WP *wp = head; wp; wp = wp->next, cnt++) {
		bool success = true;
		word_t cur_value = expr(wp->expr, &success);

		if (!success) {
			printf("Watchpoint %02d expression invalid: %s\n", wp->NO, wp->expr);
			continue;
		}

		if (cur_value != wp->value) {
			npc_state.state = NPC_STOP;
			printf("| %-12d | 0x%-10x | 0x%-10x | %s\n",
					wp->NO, (uint32_t)wp->value, (uint32_t)cur_value, wp->expr);
			wp->value = cur_value;
		} else {
			printf("| %-12d | (no change)\n", wp->NO);
			npc_state.state =  NPC_RUNNING;
		}
	}
	if (!cnt) {
		printf("(no watchpoint)\n");
	}
	return;
}
#endif
