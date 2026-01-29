
VERILATOR = verilator

all:
	@echo "Write this Makefile by your self."

sim:
	$(VERILATOR) -cc --trace --exe --build -j vsrc/example.v csrc/main.cpp
	obj_dir/Vexample
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
wave:
	#sim
	gtkwave wave.vcd
include ../Makefile
