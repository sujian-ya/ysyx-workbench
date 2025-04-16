#include <nvboard.h>
#include <Vseg.h>

static TOP_NAME dut;

void nvboard_bind_all_pins(TOP_NAME* top);

int main() {
  nvboard_bind_all_pins(&dut);
  nvboard_init();

  reset(10);

  while(1) {
    nvboard_update();
		dut.eval();
  }
}
