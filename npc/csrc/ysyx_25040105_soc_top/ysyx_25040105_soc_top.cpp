#include <common.h>
#include <sdb.h>
#include "svdpi.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vysyx_25040105_soc_top.h"

// 仿真相关的全局变量
VerilatedContext* contextp = NULL;
Vysyx_25040105_soc_top* top = NULL;
VerilatedVcdC* tfp = NULL;
extern NPCState npc_state;
extern CPU_state cpu;

void sim_init();
void sim_exit();
void single_cycle(Vysyx_25040105_soc_top &dut);
void reset (int n);

// 定义PC, 指令和寄存器
uint32_t pc      = 0x80000000;
uint32_t inst    = 0x0;
uint32_t reg[32] = {0};

extern "C" void sys_exit(int exit_state) {
    npc_state.state = exit_state ? NPC_END : NPC_ABORT;
    npc_state.halt_ret = exit_state ? 0 : 1; // 设置返回值 
    npc_state.halt_pc = top->pc; // 设置当前PC
}

void sim_get_regs() {
    memcpy(cpu.gpr, top->rf, 32 * sizeof(uint32_t));
    memcpy(reg, top->rf, 32 * sizeof(uint32_t));
}

void sim_get_pc() {
    cpu.pc = (uint32_t)top->pc;
    pc = (uint32_t)top->pc;
}

extern "C" void sim_get_inst(uint32_t* rtl_inst) {
    inst = rtl_inst[0];
}

void sim_init() {
    contextp = new VerilatedContext;
    contextp->traceEverOn(true);
    top = new Vysyx_25040105_soc_top(contextp);
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99);
    // 定义波形文件的输出路径
    tfp->open("./build/waveform.vcd");
}

void sim_exit() {
    tfp->close();
    delete top;
    delete contextp;
}

void single_cycle(Vysyx_25040105_soc_top &dut) {
    dut.clk = 0; dut.eval(); contextp->timeInc(1);tfp->dump(contextp->time());
    dut.clk = 1; dut.eval(); contextp->timeInc(1);tfp->dump(contextp->time());
    sim_get_pc();
    sim_get_regs();
}

void reset(int n) {
    top->rst = 1;
    while (n-- > 0) single_cycle(*top);
    top->rst = 0;
}
