## framework/ 仿真与工具链（预留位）

对应 Go 的 framework。写完 RTL 要验证，仿真和综合约束是必备工具。

### testbench 仿真
```
- initial 块生成时钟/复位/激励
- 用 $display / $monitor 打印（类似 fmt.Println）
- 波形 dump：$dumpfile + $dumpvars → 用 GTKWave 看波形
- 时序验证：等时钟沿采样，别和 DUT 抢沿
```

### 综合与约束
```
- 时序约束（.sdc / .cst）：告诉工具时钟频率、IO 引脚
- 建立时间 setup / 保持时间 hold：查是否收敛
- 引脚约束：IO_LOC / IO_PORT（Gowin CST 格式，见 FORGET.md）
```

### 开源工具链
```
yosys(综合) → nextpnr-himbaechel(布局布线) → gowin_pack(bitstream) → openFPGALoader(烧录)
对应 Gowin IDE 的完整流程，全命令行可脚本化
```

### 脚本化构建
```
- 一条命令从 .v 到 .fs：yosys → nextpnr → gowin_pack
- 用 Makefile 固化（类似 Go 的 Makefile 构建）
```
