## 状态机 FSM

FPGA 编程模型的本质就是"多个状态机并行跑"。一个能跑，两个能并行，N 个就是流水线/多引擎——跟软件架构里的状态机+消息+解耦是同一套思维，只是这里是物理并行。

三段式状态机是最标准的写法：**状态跳转 / 次态组合 / 输出** 三段分开。

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

## 状态机 vs 软件状态模式

```
软件（Go）                          硬件（Verilog）
type State interface{ ... }          always @(posedge clk) state <= next;
switch state { case ... }            case (state) ... endcase
goroutine 各跑各的状态机            多个 always 块各管一个状态机（物理并行）
```
