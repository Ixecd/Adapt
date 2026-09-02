## framework/ 仿真与工具链（预留位）

对应 Go 的 framework。写完 RTL 要验证，仿真和综合约束是必备工具。

### testbench 仿真
```
- initial 块生成时钟/复位/激励
- 用 $display / $monitor 打印（类似 fmt.Println）
- 波形 dump：$dumpfile + $dumpvars → 用 GTKWave 看波形
- 时序验证：等时钟沿采样，别和 DUT 抢沿
```

### 系统任务全集（仿真常用）

**打印三兄弟**（打印时机不同）：
```
$display("a=%d", a)    // 立刻打印（执行到就打印）
$monitor("a=%d", a)    // 持续监控，信号一变就打印（$monitoron/off 控制）
$strobe("a=%d", a)     // 沿后打印（当前时刻所有赋值完成后）
// 格式 %d 十进制 / %b 二进制 / %o 八进制 / %h 十六进制
// $strobe 看"本时刻最终稳定值"，与 <= 沿后更新呼应
```

**时间**：
```
$time        // 当前仿真时间（整数，按时间单位取整）
$realtime    // 当前仿真时间（实数，保留小数）——最常用 $time
```

**结束/暂停**：
```
$finish   // 结束仿真并退出
$stop     // 停止仿真，但可继续（交互模式）
$stop(n)/$finish(n)  // n=0 无输出；n=1 位置+时间；n=2 再加内存+CPU 统计
```

**读内存/随机**：
```
$readmemh("file", mem)   // 读十六进制文件到内存数组（ROM/RAM 初始化）
$readmemb("file", mem)   // 读二进制文件
$random                  // 32 位有符号随机数
$random % N              // 取模得到 0~N-1 随机数（测试激励随机化）
```

**波形（GTKWave 用）**：
```
$dumpfile("wave.vcd")    // 指定波形文件名
$dumpvars                 // 记录所有信号变化到波形
```

### 延迟建模：四种延迟（仿真用，综合忽略）

```
上升延迟 rise：输出 0→1 需要的时间
下降延迟 fall：输出 1→0 需要的时间
截止延迟 turnoff：输出 → 高阻 Z 需要的时间（三态才用）
不定态延迟 x：输出 → 未知 X 需要的时间（极少用）
```

```verilog
// 门延时 / assign 延时（仿真建模）
and #(1.2, 0.8, 0.5) g1(y, a, b);   // rise=1.2, fall=0.8, turnoff=0.5
assign #(1, 2, 3) y = a & b;         // 上升/下降/turnoff
```

- **都是仿真建模**：模拟真实器件传播延迟（让仿真波形接近硬件：0→1 要多久、1→0 要多久、→Z 要多久）
- **综合忽略**：真实延迟由门电路物理（RC/工艺）决定，不是代码写的数字
- 和 `#100`、`#(3,2,4)` 同类：仿真工具，DUT 可综合代码里不能有
- **用途层级**：RTL 仿真（行为级）不用延迟；**门级网表仿真才用**（验证时序/延迟，iCESugar 教训的那层）

### 强制赋值：assign/deassign 与 force/release（仿真调试用）

两对"过程强制赋值"，成对使用，**基本不可综合，只在 testbench 用**：

```verilog
// assign / deassign：给 reg/net 持续赋值，直到撤销
initial begin
    assign q = 1'b0;    // 持续强制 q=0（覆盖其他驱动）
    #10;
    deassign q;         // 撤销，恢复原来驱动
end

// force / release：更霸道，能覆盖 assign，直到释放
initial begin
    force q = 1'b1;     // 强制 q=1（net/reg 都能强制）
    #10;
    release q;          // 释放，恢复原驱动
end
```

```
           assign/deassign          force/release
作用对象    reg 或 net               net 或 reg
优先级      低于 force               高于 assign（force 覆盖 assign）
撤销       deassign                 release
用途       testbench 建模            testbench 调试/强制
```

- **优先级链**：`force`（最高）> 过程 `assign` > 普通赋值 > 默认驱动
- **坑**：过程 `assign`（写在 initial/always 里）和模块级 `assign`（组合逻辑，可综合）**同名但完全不同**——前者仿真强制、后者可综合组合逻辑，别混

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
