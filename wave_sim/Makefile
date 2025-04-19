TOPNAME = switch

all:
	@verilator --trace -cc vsrc/$(TOPNAME).v --exe csrc/$(TOPNAME).cpp
	@make -j -C obj_dir/ -f V$(TOPNAME).mk V$(TOPNAME)
	@./obj_dir/V$(TOPNAME)

sim: all
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!

clean:
	rm -rf obj_dir
	rm -rf waveform.vcd

include ../Makefile
