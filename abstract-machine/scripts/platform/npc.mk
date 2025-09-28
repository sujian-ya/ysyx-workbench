AM_SRCS := riscv/npc/start.S \
           riscv/npc/trm.c \
           riscv/npc/ioe.c \
           riscv/npc/timer.c \
           riscv/npc/input.c \
           riscv/npc/gpu.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

# Add include paths for RISC-V and NPC-specific headers
INC_PATH += $(AM_HOME)/am/src/riscv/npc/include
INC_PATH += $(AM_HOME)/am/src/riscv

CFLAGS    += -fdata-sections -ffunction-sections
LDSCRIPTS += $(AM_HOME)/scripts/linker.ld
LDFLAGS   += --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
NPCFLAGS += -b
NPCFLAGS += --log=$(shell dirname $(IMAGE).elf)/npc-log.txt
NPCFLAGS += $(if $(USER_ELF),--elf=$(USER_ELF),--elf=$(IMAGE).elf)

MAINARGS_MAX_LEN = 64
MAINARGS_PLACEHOLDER = The insert-arg rule in Makefile will insert mainargs here.
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=\""$(MAINARGS_PLACEHOLDER)"\"

# 添加npc的路径，增加可移植性
NPC_HOME ?= $(abspath $(dir $(AM_HOME))/npc)
# NPC_SIM = $(NPC_HOME)

insert-arg: image
	@python $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) "$(MAINARGS_PLACEHOLDER)" "$(mainargs)"

image: image-dep
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

# run: insert-arg $(NPC_SIM)
run: insert-arg
	@$(MAKE) -C $(NPC_HOME) run ARGS="$(NPCFLAGS)" IMG=$(IMAGE).bin

.PHONY: insert-arg run