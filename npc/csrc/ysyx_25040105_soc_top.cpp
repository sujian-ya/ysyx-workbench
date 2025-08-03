#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vysyx_25040105_soc_top.h"

#define PMEM_SIZE (128 * 1024 * 1024) // 内存地址：0x80000000 -- > 0x87ffffff
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

void pmem_init(const char* bin_file) {
    // 初始化内存
    for (int i = 0; i < PMEM_SIZE; i++) pmem[i] = 0;

    // 如果未提供.bin文件, 则加载默认测试指令
    if (!bin_file || strlen(bin_file) == 0) {
        printf("No .bin file provided, loading default test instructions.\n");
        pmem[0] = 0x00000013; // NOP (addi x0, x0, 0)
        pmem[1] = 0x00100093; // addi x1, x0, 1
        pmem[2] = 0x00200113; // addi x2, x0, 2
        pmem[3] = 0x00100073; // EBREAK
        return;
    }

    // 打开 .bin 文件
    FILE* fp = fopen(bin_file , "rb");
    if (!fp) {
        printf("Error: Cannot open .bin file %s\n", bin_file);
        fclose(fp);
        exit(1);
    }

    // 读取文件内容到 pmem
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // 确保文件大小是 4 的倍数 （指令按字对齐）
    if (size % 4 != 0) {
        printf("Error: .bin file size (%zu bytes) is not a multiple of 4\n", size);
        fclose(fp);
        exit(1);
    }

    size_t inst_cnt = size / 4;
    if (inst_cnt > PMEM_SIZE) {
        printf("Error: .bin file too large (%zu instructions), exceeds PMEM_SIZE (%u)\n", inst_cnt, PMEM_SIZE);
        fclose(fp);
        exit(1);
    }

    size_t read_cnt = fread(pmem, 4, inst_cnt, fp);
    if (read_cnt != inst_cnt) {
        printf("Error: Failed to read .bin file, expected %zu instructions, read %zu\n", inst_cnt, read_cnt);
        fclose(fp);
        exit(1);
    }

    printf("Loaded %zu instructions from %s to physical memory\n", inst_cnt, bin_file);
    fclose(fp);
}

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;
Vysyx_25040105_soc_top* top;

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
    // 调试输出参数
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    const char* bin_file = NULL;
    if (argc > 1) bin_file = argv[1];

    sim_init(bin_file);
    contextp->commandArgs(argc, argv);

    reset(5);
    top->inst = pmem_read(0x80000000); // 初始指令
    while (!sim_done && !Verilated::gotFinish()) {
        single_cycle(*top);
        top->inst = pmem_read(top->pc); // 下一条指令
        top->eval();
        if (top->inst == 0x00000000) break; // NOP指令，退出循环
    }

    sim_exit();
    return 0;
}
