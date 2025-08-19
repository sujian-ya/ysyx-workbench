# ps2_keyboard shift_seg.v shift_seg ALU prior_encoder selector waterfall_light switch
TOPNAME = ysyx_25040105_soc_top
# INC_PATH ?= $(abspath ./include)
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

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \))
# CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

# 定义cpu_tests的路径
BIN_FILES_PATH ?= /home/sheep/ysyx-workbench/am-kernels/tests/cpu-tests/build
# 获取该路径下所有.bin文件
BIN_FILES = $(wildcard $(BIN_FILES_PATH)/*.bin)

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""
LDFLAGS += -lreadline -ltermcap # 链接器选项，包括 readline 和 termcap

$(BIN): $(VSRCS) $(CSRCS)
	@rm -rf $(OBJ_DIR)
	@$(VERILATOR) $(VERILATOR_CFLAGS) \
		--trace \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) > /dev/null 2>&1
# 		> /dev/null 2>&1
	@$(MAKE) -j -C $(OBJ_DIR) -f V$(TOPNAME).mk
	@mkdir -p $(BUILD_DIR)  # 确保build目录存在

run: $(BIN) $(BIN_FILES)
	$(call git_commit, "sim RTL without NVBoard")
	@echo "######## Simulation beginning ########"
	@$(BIN) $(BIN_FILES) $(ARGS)
	@echo "######## Simulation finished ########"

# 生成波形文件
wave: $(BIN)
	@gtkwave $(BUILD_DIR)/waveform.vcd

all: default

clean:
	rm -rf $(BUILD_DIR)
	
.PHONY: default all clean run sim run-sim wave

include ../Makefile