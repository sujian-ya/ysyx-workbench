#ifndef __YSYX_25040105_SOC_TOP_H__
#define __YSYX_25040105_SOC_TOP_H__

#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vysyx_25040105_soc_top.h"
#include <stdint.h>

extern VerilatedContext* contextp;
extern Vysyx_25040105_soc_top* top;

void sim_init(const char* bin_file);
void sim_exit();
void reset(int n);
void single_cycle(Vysyx_25040105_soc_top &dut);

#endif
