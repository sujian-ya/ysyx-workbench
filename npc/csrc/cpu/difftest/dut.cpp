#include <dlfcn.h>
#include <memory/paddr.h>
#include <reg.h>
#include <cpu.h>
#include <utils.h>
#include <difftest-def.h>

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

#ifdef CONFIG_DIFFTEST

static bool is_skip_ref = false;
static int skip_dut_nr_inst = 0;

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref() {
  is_skip_ref = true;
  // If such an instruction is one of the instruction packing in QEMU
  // (see below), we end the process of catching up with QEMU's pc to
  // keep the consistent behavior in our best.
  // Note that this is still not perfect: if the packed instructions
  // already write some memory, and the incoming instruction in NEMU
  // will load that memory, we will encounter false negative. But such
  // situation is infrequent.
  skip_dut_nr_inst = 0;
}

// this is used to deal with instruction packing in QEMU.
// Sometimes letting QEMU step once will execute multiple instructions.
// We should skip checking until NEMU's pc catches up with QEMU's pc.
// The semantic is
//   Let REF run `nr_ref` instructions first.
//   We expect that DUT will catch up with REF within `nr_dut` instructions.
void difftest_skip_dut(int nr_ref, int nr_dut) {
  skip_dut_nr_inst += nr_dut;

  while (nr_ref -- > 0) {
    ref_difftest_exec(1);
  }
}

void init_difftest(char *ref_so_file, long img_size, int port) {
  assert(ref_so_file != NULL);

  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY);
  assert(handle);

  ref_difftest_memcpy = (void (*)(paddr_t, void*, size_t, bool))dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = (void (*)(void*, bool))dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = (void (*)(uint64_t))dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = (void (*)(uint64_t))dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  void (*ref_difftest_init)(int) = (void (*)(int))dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);

  ref_difftest_init(port);
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
}

#define gpr(i) cpu.gpr[i]
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  // 比较PC
  if (cpu.pc != ref_r->pc) {
    printf("Mismatch at pc = 0x%08x: DUT pc = 0x%08x, REF pc = 0x%08x\n",
           pc, cpu.pc, ref_r->pc);
    return false;
  }

  // 比较寄存器
  for (int i = 0; i < 32; i++) {
    if (gpr(i) != ref_r->gpr[i]) {
      printf("Mismatch at pc = 0x%08x: reg x%d, DUT = 0x%08x, REF = 0x%08x\n",
             pc, i, gpr(i), ref_r->gpr[i]);
      return false;
    }
  }
  return true;
}

#define REG_NUM 32
void diff_reg_display(CPU_state *ref_r) {
	printf("Displaying reg(diff):\n");
	printf("pc = 0x%-10x\n", (uint32_t)ref_r->pc);
	// printf("%-8s %-12s\n", "REG_NAME", "VALUE");
	int cnt = 1;
	for (int i = 0; i < REG_NUM; i++, cnt++) {
		word_t val = ref_r->gpr[i];
		if (cnt % 4 == 0) {
			printf("|%-8s 0x%-8.8x\n", regs[i], (uint32_t)val);
		} else {
			printf("|%-8s 0x%-8.8x  ", regs[i], (uint32_t)val);
		}
	}
	printf("|%-8s 0x%-8.8x  ", "mepc", (uint32_t)ref_r->mepc);
	printf("|%-8s 0x%-8.8x  ", "mstatus", (uint32_t)ref_r->mstatus);
	printf("|%-8s 0x%-8.8x  ", "mcause", (uint32_t)ref_r->mcause);
	printf("|%-8s 0x%-8.8x\n", "mtvec", (uint32_t)ref_r->mtvec);
}

static void checkregs(CPU_state *ref, vaddr_t pc) {
  if (!isa_difftest_checkregs(ref, pc)) {
    npc_state.state = NPC_ABORT;
    npc_state.halt_pc = pc;
    npc_reg_display();
    diff_reg_display(ref);
  }
}

void difftest_step(vaddr_t pc, vaddr_t npc) {
  CPU_state ref_r;

  if (skip_dut_nr_inst > 0) {
    ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
    if (ref_r.pc == npc) {
      skip_dut_nr_inst = 0;
      checkregs(&ref_r, npc);
      return;
    }
    skip_dut_nr_inst --;
    if (skip_dut_nr_inst == 0)
      panic("can not catch up with ref.pc = " FMT_WORD " at pc = " FMT_WORD, ref_r.pc, pc);
    return;
  }

  if (is_skip_ref) {
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    is_skip_ref = false;
    return;
  }

  ref_difftest_exec(1);
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);

  checkregs(&ref_r, pc);
}

void isa_difftest_attach() {
}

#else
void init_difftest(char *ref_so_file, long img_size, int port) { }
#endif
