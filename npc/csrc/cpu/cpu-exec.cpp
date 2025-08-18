#include <sdb.h>
#include <cpu.h>
#include <locale.h>
#include <utils.h>
#include <debug.h>
#include <common.h>
#include <ysyx_25040105_soc_top.h>

// 状态变量
NPCState npc_state = { .state = NPC_STOP };
// 仿真相关的全局变量和函数
extern Vysyx_25040105_soc_top* top;
extern void single_cycle(Vysyx_25040105_soc_top &dut);
extern uint32_t pmem_read(uint32_t addr);
extern void sim_exit();

uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us

static void exec_once() {
  single_cycle(*top);
  top->inst = pmem_read(top->pc);
  top->eval();
}

// NPC执行函数
static void execute(uint64_t n) {
  for(; n > 0; n--) {
    // single_cycle(*top);
    // top->inst = pmem_read(top->pc);
    // top->eval();
    exec_once();
    g_nr_guest_inst ++;
#ifdef CONFIG_WATCHPOINT
check_watchpoint();
#endif
    if(npc_state.state != NPC_RUNNING) break;
  }
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  // 执行前的状态检查
  switch (npc_state.state) {
    case NPC_END: case NPC_ABORT: case NPC_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: npc_state.state = NPC_RUNNING;
  }

  uint64_t timer_start =  get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  // 执行后的状态检查
  switch (npc_state.state) {
    case NPC_RUNNING: npc_state.state = NPC_STOP; break;
    case NPC_ABORT:
    case NPC_END:
      Log("npc: %s at pc = " FMT_WORD,
          (npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          npc_state.halt_pc);
      // fall through
    case NPC_QUIT: 
      sim_exit();
      statistic();
  }
}
