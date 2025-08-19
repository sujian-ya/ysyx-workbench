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

void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
static void exec_once() {
  // 保存当前 PC，因为 single_cycle 可能会改变它
  vaddr_t prev_pc = top->pc;

  // 执行一个周期，可能读入指令并更新 PC
  single_cycle(*top);
  top->inst = pmem_read(top->pc);
  top->eval();

  // 获取指令长度。假设指令是4字节，或者通过其他方式获取
  int ilen = 4; // RISC-V usually 4 bytes
  uint32_t inst_code = top->inst;

#ifdef CONFIG_ITRACE
  char logbuf[128]; // 本地日志缓冲区
  char *p = logbuf;

  // 1. 打印 PC 地址
  p += snprintf(p, sizeof(logbuf), FMT_WORD ":", prev_pc);

  // 2. 打印机器码
  uint8_t *inst_bytes = (uint8_t *)&inst_code;
  // RISC-V 通常逆序打印机器码，以符合小端序的可读性
  for (int i = ilen - 1; i >= 0; i--) {
      p += snprintf(p, 4, " %02x", inst_bytes[i]);
  }

  // 3. 添加对齐空格
  int ilen_max = 4; // For RISC-V
  int space_len = ilen_max * 3 + 1 - (ilen * 3);
  if (space_len < 0) space_len = 0;
  memset(p, ' ', space_len);
  p += space_len;

  // 4. 反汇编并打印到日志缓冲区
  disassemble(p, sizeof(logbuf) - (p - logbuf), prev_pc, inst_bytes, ilen);

  // 5. 打印日志
  _Log("%s\n", logbuf);
#endif
#ifdef CONFIG_WATCHPOINT
  check_watchpoint();
#endif
}

// NPC执行函数
static void execute(uint64_t n) {
  for(; n > 0; n--) {
    exec_once();
    g_nr_guest_inst ++;
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
