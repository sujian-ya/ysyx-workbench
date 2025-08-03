# ps2_keyboard shift_seg.v shift_seg ALU prior_encoder selector waterfall_light switch
TOPNAME = ysyx_25040105_soc_top
NXDC_FILES = constr/$(TOPNAME).nxdc
INC_PATH ?=

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --x-initial fast --noassert

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))

# constraint file
SRC_AUTO_BIND = $(abspath $(BUILD_DIR)/auto_bind.cpp)
$(SRC_AUTO_BIND): $(NXDC_FILES)
	@python3 $(NVBOARD_HOME)/scripts/auto_pin_bind.py $^ $@

# project source
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "$(TOPNAME).c" -or -name "$(TOPNAME).cc" -or -name "$(TOPNAME).cpp")
CSRCS += $(SRC_AUTO_BIND)

# rules for NVBoard
include /home/sheep/ysyx-workbench/nvboard/scripts/nvboard.mk

# rules for verilator
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

$(BIN): $(VSRCS) $(CSRCS) $(NVBOARD_ARCHIVE)
	@rm -rf $(OBJ_DIR)
	@$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN)) > /dev/null 2>&1

all: default

run: $(BIN)
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@$^

########## Wave Simulation
# 定义cpu_tests的路径
BIN_FILES_PATH ?= /home/sheep/ysyx-workbench/am-kernels/tests/cpu-tests/build
# 获取该路径下所有.bin文件
BIN_FILES = $(wildcard $(BIN_FILES_PATH)/*.bin)

BIN_SIM = $(BUILD_DIR)/$(TOPNAME)_sim
CSRCS_SIM = $(filter-out $(SRC_AUTO_BIND), $(CSRCS))  # 排除自动绑定的文件

$(BIN_SIM): $(VSRCS) $(CSRCS_SIM)
	@rm -rf $(OBJ_DIR)
	@$(VERILATOR) $(VERILATOR_CFLAGS) \
		--trace \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS) -I$(abspath ./vsrc)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN_SIM)) 
# 		> /dev/null 2>&1
	@$(MAKE) -j -C $(OBJ_DIR) -f V$(TOPNAME).mk
	@mkdir -p $(BUILD_DIR)  # 确保build目录存在

sim: $(BIN_SIM) $(BIN_FILES)
	$(call git_commit, "sim RTL without NVBoard")
	@echo "######## Simulation beginning ########"
	@$(BIN_SIM)  $(BIN_FILES)
	@echo "######## Simulation finished ########"

wave: $(BIN_SIM)
	@gtkwave $(BUILD_DIR)/waveform.vcd
########

clean:
	rm -rf $(BUILD_DIR)
	
.PHONY: default all clean run sim run-sim wave

include ../Makefile
