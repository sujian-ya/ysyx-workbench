LIBCAPSTONE = tools/capstone/repo/libcapstone.so.5
INC_PATH += $(abspath tools/capstone/repo/include)
csrc/utils/disasm.cpp: $(LIBCAPSTONE)
$(LIBCAPSTONE):
	$(MAKE) -C tools/capstone
