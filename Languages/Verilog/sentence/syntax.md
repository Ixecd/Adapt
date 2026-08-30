## module 与端口

模块是 Verilog 的基本单元，类似 Go 里的 package + func 组合。端口分 input/output/inout。

```verilog
module counter #(parameter W = 8) (
    input  wire       clk,    // 时钟
    input  wire       rst_n,  // 低有效复位
    output reg [W-1:0] cnt    // 计数值，reg 类型（always 块里赋值）
);

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            cnt <= 0;
        else
            cnt <= cnt + 1'b1;
    end

endmodule
```

- `wire`：组合逻辑连线，`assign` 赋值；`reg`：时序逻辑变量，`always` 块里赋值
- 参数化 `#(parameter W = 8)` 类似模板/泛型，实例化时可覆盖 `counter #(.W(16)) u(...)`

## 数据类型

```verilog
wire        a;          // 1 bit 线网
wire [7:0]  bus;        // 8 bit 总线
reg         r;          // 1 bit 寄存器
reg [31:0]  data;       // 32 bit 寄存器
integer     i;          // 整型（多用于 for 循环/仿真）
parameter   W = 8;      // 常量，类似 const
localparam  W2 = W * 2; // 局部常量，不可被外部覆盖
```

- 位宽用 `[MSB:LSB]` 表示，如 `[7:0]` 是低 8 位
- 数值写法：`8'hFF`（十六进制）、`4'b1010`（二进制）、`8'd255`（十进制）、`1'b1`/`1'b0`/`1'bx`/`1'bz`

## assign 连续赋值

`assign` 是组合逻辑，等号右侧变化立即反映到左侧，类似 wire 的"接线"。

```verilog
wire a, b;
wire y;

assign y = a & b;   // 与门
assign y = ~a;      // 非门
assign y = a | b;   // 或门
assign y = a ^ b;   // 异或
```

## always 块

`always` 是最核心的块，分组合（电平敏感）和时序（边沿敏感）两种。

### 组合逻辑（电平敏感 @*）

```verilog
// @* 表示"所有输入变化都触发"，不用手动列端口
reg [3:0] y;
always @* begin
    case (sel)
        2'd0: y = 4'b0001;
        2'd1: y = 4'b0010;
        default: y = 4'b0000;
    endcase
end
```

### 时序逻辑（边沿敏感 posedge/negedge）

```verilog
reg [3:0] q;
always @(posedge clk or negedge rst_n) begin
    if (!rst_n)
        q <= 4'd0;
    else
        q <= d;   // 在时钟上升沿采样
end
```

## 阻塞赋值 = 与非阻塞赋值 <=

这是 Verilog 最容易踩的坑，相当于"单线程顺序" vs "并行同时"。

```verilog
// 阻塞赋值 =：顺序执行（类似普通赋值），用于组合逻辑
always @* begin
    a = b;
    c = a;   // c 拿到的是 b（已经更新）
end

// 非阻塞赋值 <=：并行执行（所有右侧同时采样），用于时序逻辑
always @(posedge clk) begin
    a <= b;
    c <= a;   // c 拿到的是 a 的旧值（采样瞬间的值），不是 b
end
```

**铁律**：时序逻辑用 `<=`，组合逻辑用 `=`。混用会导致仿真和综合结果不一致。

## 参数与实例化

```verilog
// 定义
module mux2 #(parameter W = 8) (
    input  [W-1:0] a, b,
    input          sel,
    output [W-1:0] y
);
    assign y = sel ? a : b;
endmodule

// 实例化（类似调用）
wire [15:0] ya, yb;
wire        s;
wire [15:0] y16;

mux2 #(.W(16)) u_mux (
    .a(ya), .b(yb), .sel(s), .y(y16)
);
```

- `.name(port)` 是**按名字连接**，比按位置安全，类似 Go 的命名参数/结构体字段
- `#(.W(16))` 覆盖默认参数，类似泛型实例化
