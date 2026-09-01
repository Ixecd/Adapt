## function/ 常用模块（预留位）

学课时边学边补，每个模块配可综合代码。这里是计划清单：

### 计数器 counter
最基本时序模块，任何分频/定时都基于它。
```
- 普通 N bit 计数器
- 使能计数（ce）
- 计数到值清零/翻转
```

### 分频器 divider
用计数器把高频时钟分频成低频。
```
- 偶数分频（2/4/8...）：计数器计数翻转
- 奇数分频（3/5/7...）：需两个沿（posedge+negedge）组合
- 占空比 50% 的奇数分频是常见考点
```

### 流水线 pipeline
把组合逻辑切开插寄存器，提升时钟频率（类似 Go 里"用空间换时间"）。
```
- 组合链太长 → 时序不收敛
- 插一级寄存器 → 延迟加一拍，吞吐提升
```

### 移位寄存器 shift
```
- 串入并出 SIPO（串口接收的基础）
- 并入串出 PISO（串口发送的基础）
- LFSR（伪随机数，测试用）
```

### 环形缓冲区 ring buffer / FIFO
底层 = **数组 + 读写指针**，指针位宽溢出自动回绕实现"环形"：

```verilog
reg [7:0] buf [0:15];     // 16 槽数组
reg [3:0] wr_ptr, rd_ptr; // 写/读指针

always @(posedge clk) begin
    if (wr_en) begin
        buf[wr_ptr] <= din;
        wr_ptr <= wr_ptr + 1;    // 4 位溢出自动回 0 = 环形
    end
end
always @(posedge clk) begin
    if (rd_en) begin
        dout <= buf[rd_ptr];
        rd_ptr <= rd_ptr + 1;
    end
end
```

- **回绕靠位宽溢出**：4 位指针 +1 到 16 自动回 0，不用判断
- **满/空**：`wr==rd` 空；满 = 加 full 标志 或 留一空槽（`wr+1==rd`）
- **= FIFO 的底层实现**：异步 FIFO（不同时钟）→ 指针格雷码 + 打拍同步（见 concurrency）

### 序列信号发生器
周期输出预定 0/1 序列（如 `1011011` 循环）：
```verilog
reg [6:0] seq = 7'b1011011;
always @(posedge clk)
    seq <= {seq[5:0], seq[6]};   // 循环左移
assign out = seq[6];             // 每拍输出最高位
```
- 移位寄存器存序列循环移出 = 精确复现；LFSR = 伪随机（测试用）
- 本质 = 状态机（每个状态 = 序列位置），"带时钟的 DP"的应用
- 与 Feelings 相关：tVNS 刺激波形（脉冲/频率）就是序列发生器控制
