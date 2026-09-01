## 状态机 FSM

FPGA 编程模型的本质就是"多个状态机并行跑"。一个能跑，两个能并行，N 个就是流水线/多引擎——跟软件架构里的状态机+消息+解耦是同一套思维，只是这里是物理并行。

### 为什么需要状态机：硬件天然并行，串行要靠它造

**硬件为什么天然并行**：芯片一上电，**所有电路同时得电、同时开始工作**——没有"先启动 A 再启动 B"。所有 always 块、组合逻辑、触发器从加电那一刻就同时运转。并行是**物理必然**（加电同时），不是设计出来的。

**所以串行是"反物理"的**：硬件里没有"顺序执行"。要按顺序做事，**必须人为制造顺序**——这就是状态机：
- 状态机在"多个状态之间按顺序跳转"，用时钟一拍一拍推进
- **状态机 = 在并行硬件里人为模拟串行**（S0 做 A，S1 做 B，时钟驱动跳转）

**和软件正好相反**：
```
软件（CPU）：天生串行（一条条指令），要并行得靠多线程/协程（人为造并行）
硬件（FPGA）：天生并行（加电全开），要串行得靠状态机（人为造串行）
```

**注意**：状态机的"顺序"也是假顺序——硬件每个时钟周期全在跑，只是"转移条件"让人感觉按顺序走（像流水线：物理全并行，数据看似按序流动）。

### 三段式（推荐）

```verilog
module fsm (
    input  wire clk,
    input  wire rst_n,
    input  wire start,
    input  wire done,
    output wire busy
);

    // 状态编码（localparam 是局部常量，可综合）
    localparam IDLE = 2'd0;
    localparam WORK = 2'd1;
    localparam DONE = 2'd2;

    reg [1:0] state, next_state;

    // 段1：状态跳转（时序，非阻塞）
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            state <= IDLE;
        else
            state <= next_state;
    end

    // 段2：次态计算（组合，阻塞）
    always @* begin
        next_state = state;
        case (state)
            IDLE: if (start) next_state = WORK;
            WORK: if (done)  next_state = DONE;
            DONE: next_state = IDLE;
            default: next_state = IDLE;
        endcase
    end

    // 段3：输出（组合，看状态即可）
    assign busy = (state == WORK);

endmodule
```

- 段2 里 `next_state = state;` 作为默认值（保持原状态），case 里只写转移条件，避免漏写产生锁存器
- 三段式的优点：状态和输出分离，好维护、好调试（类似软件里"状态机类"分离状态与行为）

### 为什么 Verilog 是多 case，不是软件的一个 switch

```
软件（Pivot）：一个 switch(state) 全写完（串行，函数内部耦合）
Verilog 三段式：多个独立 case，各放不同 always（并行，解耦）
  always @* case(state) next_state = ...;   // 状态转换 case（算下一步去哪）
  always @* case(state) output = ...;        // 输出 case（算现在输出啥，独立！）
→ 两个 case 同时跑（并行），改输出不影响次态
```

**核心原因**：硬件天然并行 → 多个 case 同时执行、互不干扰 → 能把"次态/输出"拆成独立 case。这就是三段式比一段式好的根本原因：**并行让你能拆，拆了各段独立演进**。同一套状态机逻辑，软件（串行）被迫单 switch 耦合，硬件（并行）天然多 case 解耦。

### 二段式 / 一段式

```verilog
// 二段式：跳转+次态合在一起，输出单独
// 一段式：全部塞进一个 always，输出和状态耦合，不好改，不推荐
```

### 状态编码选择

| 编码方式 | 说明 |
|---------|------|
| 二进制编码 | 面积小，状态多时省寄存器，译码稍复杂 |
| 格雷码 | 相邻状态只有 1 bit 翻转，抗毛刺，适合状态连续跳转 |
| 独热码 | 每个状态一个 bit，译码快、时序好，适合状态少（<8） |

```verilog
// 独热码示例：状态多但跳转简单时，比较器变成简单或门，时序更稳
localparam IDLE = 4'b0001;
localparam WORK = 4'b0010;
localparam DONE = 4'b0100;
```

### Moore vs Mealy：输出依赖什么

- **Moore**：输出**只看当前状态**（状态变输出才变，沿后更新，稳定无毛刺）
- **Mealy**：输出看**状态 + 当前输入**（输入一变输出立刻变，响应快但可能毛刺）

```verilog
// Moore：输出只 case 状态
assign out = (state == S1) ? 1 : 0;          // 只依赖 state

// Mealy：输出还看输入
assign out = (state == S1 && in) ? 1 : 0;    // 依赖 state + in
```

**判据就一条**：`assign 输出的表达式里有没有"输入"`——有 = Mealy，没有 = Moore。

| | Moore | Mealy |
|--|-------|-------|
| 输出依赖 | 只看状态 | 状态 + 输入 |
| 变化时机 | 状态更新后（沿后） | 输入一变就变（组合） |
| 响应 | 慢一拍 | 立即 |
| 稳定性 | 稳（无毛刺） | 输入抖动会反映到输出 |
| 状态数 | 可能多 | 可能少（输出靠输入区分） |

**选择标准**：要输出稳定（安全/指示）→ Moore；要输入快速响应（协议/接收）→ Mealy。
- 你 Feelings 的安全熔断/看门狗 → Moore（输出稳，不受输入毛刺干扰）
- UART 接收/串行协议 → Mealy（输入变化要立刻处理，省状态）

### 实例：连续 3 个 1 检测器（状态方程 + 输出方程）

**三大方程**：
```
Q1^(n+1) = Q0     ← 状态方程1：下一时刻 Q1 = 当前 Q0
Q0^(n+1) = X      ← 状态方程2：下一时刻 Q0 = 当前 X（输入移入）
Z = Q1·Q0·X       ← 输出方程：组合判断（三个都 1 → Z=1）
```

**这是什么**：每个时钟沿把 X 移进 Q0、旧 Q0 移进 Q1 = **移位寄存器**；Z 检测"连续三个 1"。

```verilog
// 状态方程（时序，<=）
always @(posedge clk) begin
    Q1 <= Q0;    // Q1^(n+1) = Q0
    Q0 <= X;     // Q0^(n+1) = X
end
// 输出方程（组合）
assign Z = Q1 & Q0 & X;
```

**逐拍推演**：
```
拍0: X=1 → Q0=1,Q1=0 → Z=0（1个1）
拍1: X=1 → Q0=1,Q1=1 → Z=1（连续3个1！）
拍2: X=1 → Z=1（1111 最后三个仍全1）
拍3: X=0 → Z=0（立刻变0，不锁存）
```

**关键：Z 是组合逻辑，不会"三个1后重置"**：
- Z = 当下三个值的与 → 输入变输出立刻变
- 不锁存、不重置（那是"锁存型检测器"要加额外状态）
- 这是"滑动窗口组合判断"，不是"事件锁存"

**和 DP 同构（带时钟的 DP）**：状态方程 = 递推（每个沿迭代一次），Q1/Q0 是记忆、X 是当前输入，跟 `dp[i]=f(dp[i-1],x[i])` 同构。

### pre_state：可回溯的状态机（类比双向链表）

### 避坑：次态计算里不要写 `next_state = x`

```verilog
// 错：next_state 设成 x（未知态）
always @(current_state) begin
    next_state = x;    // ← x 是"未知"不是"占位"！污染后续
    case (current_state) ...
end

// 对：默认"保持当前状态"，不是 x
always @* begin
    next_state = current_state;   // 默认：保持（用 current_state，不是 x）
    case (current_state)
        IDLE: if (start) next_state = WORK;
        default: next_state = IDLE;   // 非法状态防御
    endcase
end
```

- `x` 是"未知"不是"默认值"：仿真 x 会传染（没被 case 命中的状态变 x → 状态机跑飞），综合可能不确定
- **组合 always 的默认值永远用"保持当前状态"（current_state），绝不用 x**
- 这就是"段2 里 next_state = state 作为默认值"的标准写法

```verilog
reg [1:0] pre_state;    // 前一个状态（类似链表的 prev）
reg [1:0] state;        // 当前状态
reg [1:0] next_state;   // 次态（类似链表的 next）

always @(posedge clk) begin
    pre_state <= state;       // 记住上一个
    state <= next_state;      // 进入次态
end
```

**pre_state 的用途**：
- **回溯**：状态异常时知道"从哪来的"，可回滚恢复（安全）
- **上下文**：行为依赖"怎么走到这"（= 分支预测的"分支间相关性"，Yeh & Patt）
- **审计**：状态切换历史（Pivot 的 `Transition{From,To,Reason}` 就是它，记录 from→to→为什么）

**和双向链表的区别**：链表 prev/next 是**固定结构**（连接不变），状态机是**动态转移**（next 由条件决定）。类比帮理解"有前有后"，但状态机的 next 是"条件决定"不是"结构固定"。

**一句话**：`current_state/next_state` 像双向链表的 `prev/next`——加个 `pre_state` 让状态机**可回溯**（异常回滚、上下文判断、审计日志），对安全/审计类系统（Pivot、Feelings 熔断）特别有价值。

## 状态机 vs 软件状态模式

```
软件（Go）                          硬件（Verilog）
type State interface{ ... }          always @(posedge clk) state <= next;
switch state { case ... }            case (state) ... endcase
goroutine 各跑各的状态机            多个 always 块各管一个状态机（物理并行）
```
