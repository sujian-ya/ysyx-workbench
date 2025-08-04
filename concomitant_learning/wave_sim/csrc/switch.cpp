#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "Vswitch.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;
Vswitch* top;

void sim_init() {
	contextp = new VerilatedContext;
	top = new Vswitch(contextp);
	tfp = new VerilatedVcdC;
	contextp->traceEverOn(true);
	top->trace(tfp, 99);
	tfp->open("waveform.vcd");
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

int main(int argc, char** argv)
{
	sim_init();
	contextp->commandArgs(argc, argv);

	while(!Verilated::gotFinish())
 	{
		int a = rand() & 1;
		int b = rand() & 1;
		top->a = a;
		top->b = b;
		top->eval();
		printf("a = %d, b = %d, f = %d\n", a, b, top->f);
		assert(top->f == (a ^ b));
		sim_step();
	}
	sim_exit();
	return 0;
}
