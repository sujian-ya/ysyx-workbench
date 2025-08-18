#include <dlfcn.h>
#include <capstone/capstone.h>
#include <common.h>
#include <config.h>

static size_t (*cs_disasm_dl)(csh handle, const uint8_t *code,
    size_t code_size, uint64_t address, size_t count, cs_insn **insn);
static void (*cs_free_dl)(cs_insn *insn, size_t count);

static csh handle;

void init_disasm() {
  void *dl_handle;
  dl_handle = dlopen("tools/capstone/repo/libcapstone.so.5", RTLD_LAZY);
  assert(dl_handle);

  // 显式类型转换
  cs_err (*cs_open_dl)(cs_arch arch, cs_mode mode, csh *handle) =
      (cs_err (*)(cs_arch, cs_mode, csh*))dlsym(dl_handle, "cs_open");
  assert(cs_open_dl);

  // 显式类型转换
  cs_disasm_dl = (size_t (*)(csh, const uint8_t*, size_t, uint64_t, size_t, cs_insn**))dlsym(dl_handle, "cs_disasm");
  assert(cs_disasm_dl);

  // 显式类型转换
  cs_free_dl = (void (*)(cs_insn*, size_t))dlsym(dl_handle, "cs_free");
  assert(cs_free_dl);

  cs_arch arch = MUXDEF(CONFIG_ISA_x86,          CS_ARCH_X86,
                    MUXDEF(CONFIG_ISA_mips32, CS_ARCH_MIPS,
                    MUXDEF(CONFIG_ISA_riscv,  CS_ARCH_RISCV,
                    MUXDEF(CONFIG_ISA_loongarch32r,   CS_ARCH_LOONGARCH, (cs_arch)-1))));

  // 显式类型转换
  cs_mode mode = (cs_mode)MUXDEF(CONFIG_ISA_x86,          CS_MODE_32,
                                  MUXDEF(CONFIG_ISA_mips32, CS_MODE_MIPS32,
                                  MUXDEF(CONFIG_ISA_riscv,  (cs_mode)(MUXDEF(CONFIG_ISA64, CS_MODE_RISCV64, CS_MODE_RISCV32) | CS_MODE_RISCVC),
                                  MUXDEF(CONFIG_ISA_loongarch32r,   CS_MODE_LOONGARCH32, (cs_mode)-1))));

  int ret = cs_open_dl(arch, mode, &handle);
  assert(ret == CS_ERR_OK);

#ifdef CONFIG_ISA_x86
  cs_err (*cs_option_dl)(csh handle, cs_opt_type type, size_t value) = (cs_err (*)(csh, cs_opt_type, size_t))dlsym(dl_handle, "cs_option");
  assert(cs_option_dl);

  ret = cs_option_dl(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);
  assert(ret == CS_ERR_OK);
#endif
}

void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte) {
	cs_insn *insn;
	size_t count = cs_disasm_dl(handle, code, nbyte, pc, 0, &insn);
  assert(count == 1);
  int ret = snprintf(str, size, "%s", insn->mnemonic);
  if (insn->op_str[0] != '\0') {
    snprintf(str + ret, size - ret, "\t%s", insn->op_str);
  }
  cs_free_dl(insn, count);
}
