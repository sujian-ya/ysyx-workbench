#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vysyx_25040105_soc_top.h"

#define PMEM_SIZE (1024 * 1024)
uint32_t pmem[PMEM_SIZE];

// 仿真退出标志
static int sim_done = 0;

// DPI-C导出的函数，供Verilog调用
extern "C" void ebreak_handler() {
    printf("EBREAK detected, stopping simulation\n");
    sim_done = 1; // 设置退出标志
}

uint32_t pmem_read(uint32_t addr) {
    assert((addr & 0x3) == 0); // 地址必须4字节对齐，直接位数比较加快处理速度
    uint32_t index = (addr - 0x80000000) >> 2;
    assert(index < PMEM_SIZE); // 确保地址在范围内
    return pmem[index];
}

void pmem_init() {
    for (int i = 0; i < PMEM_SIZE; i++) pmem[i] = 0;
    // 测试指令，包括ebreak
    pmem[0] = 0x00000013; // NOP (addi x0, x0, 0)
    pmem[1] = 0x00100093; // addi x1, x0, 1
    pmem[2] = 0x00200113; // addi x2, x0, 2
    pmem[3] = 0x00100073; // EBREAK
}

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;
Vysyx_25040105_soc_top* top;

void sim_init() {
    contextp = new VerilatedContext;
    contextp->traceEverOn(true);
    
    top = new Vysyx_25040105_soc_top(contextp);
    
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99);
    tfp->open("waveform.vcd");
    
    pmem_init();
}

void sim_step() {
    contextp->timeInc(1);
    tfp->dump(contextp->time());
}

void sim_exit() {
    tfp->close();
    delete top;
    delete contextp;
}

static void single_cycle(Vysyx_25040105_soc_top &dut) {
    dut.clk = 0; dut.eval(); sim_step();
    dut.clk = 1; dut.eval(); sim_step();
}

static void reset(int n) {
    top->rst = 1;
    while (n-- > 0) single_cycle(*top);
    top->rst = 0;
}

int main(int argc, char** argv) {
    sim_init();
    contextp->commandArgs(argc, argv);

    reset(5);
    top->inst = pmem_read(0x80000000); // 初始指令
    while (!sim_done && !Verilated::gotFinish()) {
        single_cycle(*top);
        top->inst = pmem_read(top->pc); // 下一条指令
        top->eval();
    }

    sim_exit();
    return 0;
}