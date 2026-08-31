# routine_test

## Verilog

硬件描述语言（HDL 的一种，VHDL 是另一种；HDL 是总称）。开源工具链 yosys/nextpnr/apycula/gowin_pack 只支持 Verilog。按主题分目录：

```
sentence/        语法基础：module/端口/数据类型/assign/always/阻塞非阻塞/参数实例化
data/            信号态：0/1/x/z、上拉下拉电阻、逻辑电平阈值
characteristics/ 核心特性：状态机 FSM、触发器串联（RTL 本质，沿前采样延迟一拍）
function/        常用模块：计数器/分频/流水线/移位寄存器
concurrency/     并行与跨时钟域：CDC/亚稳态/异步FIFO（最易翻车）
io/              接口协议：UART/SPI/I2C/按键消抖
framework/       仿真与工具链：testbench/综合约束/开源流程/脚本化
advanced/        高级主题：PLL/BRAM/原语/高速接口/优化
others/          核心概念：可综合、选型与认知（FPGA vs MCU）、脑机演进
```

关键认知（对应各 md）：
- **Verilog 和 C++ 的本质区别 = 多了个时钟沿**：Verilog 逼你显式处理"沿前（采样）沿后（更新）"，高级语言把这个"沿前沿后"封装进编译器/运行时。所有 Verilog 特性（并行、打拍、触发器、组合vs时序、流水线）都是"时钟沿显式"这一个维度的展开。从 C++ 到 Verilog 不是学新语法，是学会"面对物理时间维度"。（延伸：状态机 = 带时钟的 DP，高级语言自动忽略时钟）
- Verilog 是"硬件界的汇编"（样板多、效率低），但啰嗦逼你想清每个信号时序——理解工具，值得学扎实；高级抽象（SystemVerilog/Chisel/HLS）是提效工具
- 学 Verilog 不会忘 C++/Go：always 并行 = goroutine；wire/reg 数据流 = 数据流理解；FSM = 状态模式。只会手生，思维更强
- FPGA 本质 = 多个状态机并行跑；选型不是"MCU vs FPGA 谁厉害"，是看活儿多大（异构+按需，NVIDIA 的教训）
- 软件出身学硬件是最优路径：先软件思维设计系统（地基），再用 FPGA 做高级执行形态
