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
		printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
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
		printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
		printf("Invalid expression '%s'.\n", args);
		free_wp(new_node);
		return;
	}

	printf("%s\n", ANSI_FMT("Creating watchpoint:", ANSI_FG_LIGHTPINK));
	printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
	printf("%-14s %-14s %-14s %-14s\n", "watchpoint", "uint32_t", "int32_t", "expression");
	printf("| %-12d | 0x%-10x | %-12d | %s\n",
			new_node->NO, (uint32_t)new_node->value, (int32_t)new_node->value, new_node->expr);
}

void delete_watchpoint(int no) {
	WP *current = head;

	printf("%s\n", ANSI_FMT("Deleting watchpoint:", ANSI_FG_LIGHTPINK));
	printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
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
		printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
		printf("No watchpoint.\n");
		return;
	}

	WP *current = head;
	printf("%s\n", ANSI_FMT("Displaying watchpoint:", ANSI_FG_LIGHTPINK));
	printf("pc = %s%08x%s\n", ANSI_BG_LIGHTPINK, cpu.pc, ANSI_NONE);
	printf("%-14s %-14s %-14s %-14s\n", "watchpoint", "uint32_t", "int32_t", "expression");
	while (current != NULL) {
		printf("| %-12d | 0x%-10x | %-12d | %s\n",
			 	current->NO, (uint32_t)current->value, (int32_t)current->value, current->expr);
		current = current->next;
	}
}

void check_watchpoint() {
#ifdef CONFIG_WATCHPOINT
    if (npc_state.state == NPC_END) {
        return;
    }

    int total_cnt = 0;                // 总监视点数量
    int changed_cnt = 0;              // 发生变化的监视点数量
    WP* changed_wps[NR_WP];              // 存储发生变化的监视点（可根据需要调整大小）
    word_t old_values[NR_WP], new_values[NR_WP];  // 存储对应的值变化

    // 第一次遍历：收集所有发生变化的监视点
    for (WP *wp = head; wp; wp = wp->next, total_cnt++) {
        bool success = true;
        word_t cur_value = expr(wp->expr, &success);

        if (!success) {
            continue;  // 跳过表达式无效的监视点
        }

        if (cur_value != wp->value) {
            // 记录变化的监视点信息
            if (changed_cnt < NR_WP) {  // 防止数组越界
                changed_wps[changed_cnt] = wp;
                old_values[changed_cnt] = wp->value;
                new_values[changed_cnt] = cur_value;
                changed_cnt++;
            }
        }
    }

    // 只有当有监视点发生变化时才输出信息
    if (changed_cnt > 0) {
        printf("\nChecking watchpoint:\n");
        printf("%-12s %-12s %-12s %-12s\n", "PC", "Watchpoint", "Old_value", "New_value");
        
        // 输出所有变化的监视点信息
        npc_state.state = NPC_STOP;
        for (int i = 0; i < changed_cnt; i++) {
            WP* wp = changed_wps[i];
            printf("0x%-10x %-12d 0x%-10x 0x%-10x\n",
                    cpu.pc, wp->NO, 
                    (uint32_t)old_values[i], 
                    (uint32_t)new_values[i]);
            wp->value = new_values[i];  // 更新监视点的值
        }
    } else {
        // 没有变化时保持运行状态，不输出任何信息
        npc_state.state = NPC_RUNNING;
    }

    return;
#endif
}
