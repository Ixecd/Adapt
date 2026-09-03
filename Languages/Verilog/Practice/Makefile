# Verilog 仿真 Makefile
# 用法：
#   make TOP=counter      编译 + 仿真 counter.v + counter_tb.v，生成 counter.vcd
#   make wave TOP=counter 编译 + 仿真 + 用 surfer 打开波形
#   make clean            清理生成文件
#
# 文件约定：
#   <name>.v      模块
#   <name>_tb.v   testbench
#   <name>.vcd    波形（自动生成）

TOP ?= counter
SIM ?= vvp
WAVEVIEWER ?= surfer

VSRC = $(TOP).v $(TOP)_tb.v
VVP  = $(TOP).vvp
VCD  = $(TOP).vcd

.PHONY: all sim wave clean

all: sim

sim: $(VVP)
	vvp $(VVP)

$(VVP): $(VSRC)
	iverilog -o $(VVP) $(VSRC)

wave: sim
	$(WAVEVIEWER) $(VCD) &

clean:
	rm -f *.vvp *.vcd *.vpd
