#include <common.h>
#include <utils.h>
#include <pmem.h>
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

void sim_init(const char* bin_file);
void sim_exit();
void single_cycle(Vysyx_25040105_soc_top &dut);
void reset (int n);

// 定义PC和寄存器
uint32_t pc = 0x80000000;
uint32_t reg[32] = {0};

extern "C" void sys_exit(int exit_state) {
    npc_state.state = exit_state ? NPC_END : NPC_ABORT;
    npc_state.halt_ret = exit_state ? 0 : 1; // 设置返回值 
    npc_state.halt_pc = top->pc; // 设置当前PC
}

extern "C" void sim_get_regs(uint32_t rf[32]) {
    memcpy(reg, rf, 32 * sizeof(uint32_t));
}

extern "C" void sim_get_pc(uint32_t* rtl_pc) {
    pc = rtl_pc[0];
}

void sim_init(const char* bin_file) {
    contextp = new VerilatedContext;
    contextp->traceEverOn(true);
    top = new Vysyx_25040105_soc_top(contextp);
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99);
    // 定义波形文件的输出路径
    tfp->open("./build/waveform.vcd");
    
    pmem_init(bin_file); // 初始化内存，加载指令
}

void sim_exit() {
    tfp->close();
    delete top;
    delete contextp;
}

void single_cycle(Vysyx_25040105_soc_top &dut) {
    dut.clk = 0; dut.eval(); contextp->timeInc(1);tfp->dump(contextp->time());
    dut.clk = 1; dut.eval(); contextp->timeInc(1);tfp->dump(contextp->time());
    // 更新全局pc变量的值为当前top的pc(第二种获取pc方法)
    // pc = (uint32_t)top->pc;
}

void reset(int n) {
    top->rst = 1;
    while (n-- > 0) single_cycle(*top);
    top->rst = 0;
}
