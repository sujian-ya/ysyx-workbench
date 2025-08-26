TOPNAME = ysyx_25040105_soc_top
INC_PATH := $(shell find $(abspath ./include) -type d)

# Include all filelist.mk to merge file lists
FILELIST_MK = $(shell find -L ./csrc -name "filelist.mk")
include $(FILELIST_MK)

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --x-initial fast --noassert

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

# DIFFTEST_LDFLAGS = $(NEMU_HOME)/build/riscv32-nemu-interpreter-so
DIFF_REF_SO = $(NEMU_HOME)/build/riscv32-nemu-interpreter-so
ARGS_DIFF = --diff=$(DIFF_REF_SO)
override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt
override ARGS += $(ARGS_DIFF)
IMG ?= 

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \))

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
# LDFLAGS += -lreadline -ltermcap $(DIFFTEST_LDFLAGS)# 链接器选项，包括 readline 和 termcap
LDFLAGS += -lreadline -ltermcap

$(BIN): $(VSRCS) $(CSRCS)
	@rm -rf $(OBJ_DIR)
	@$(VERILATOR) $(VERILATOR_CFLAGS) \
		--trace \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))
# 		> /dev/null 2>&1
	@$(MAKE) -j -C $(OBJ_DIR) -f V$(TOPNAME).mk
	@mkdir -p $(BUILD_DIR)  # 确保build目录存在

run: $(BIN)
	$(call git_commit, "sim RTL without NVBoard")
	@echo "######## Simulation beginning ########"
	@$(BIN) $(ARGS) $(IMG)
	@echo "######## Simulation finished ########"

# 生成波形文件
wave: $(BIN)
	@gtkwave $(BUILD_DIR)/waveform.vcd

all: default

clean:
	rm -rf $(BUILD_DIR)
	
.PHONY: default all clean run sim run-sim wave

include ../Makefile