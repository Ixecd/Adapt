## 仿真打印三兄弟：$display / $monitor / $strobe

testbench 打印工具，区别在"打印时机"：

### $display（立刻打印）
```verilog
$display("a=%d", a);   // 执行到这条，立刻打印当前值（一次性）
```

### $monitor（持续监控打印）
```verilog
$monitor("a=%d b=%d", a, b);   // 注册监控：a 或 b 一变就自动打印
$monitoron;   // 开启
$monitoroff;  // 关闭
// 独立指令，持续监控信号，一变就打
```

### $strobe（沿后打印）
```verilog
$strobe("a=%d", a);   // 当前时刻所有赋值完成后才打印
```
- **对**：在当前时刻所有 `<=` 更新完成后才打印（看"最终稳定值"）
- **注意**：`b/o/h` 是 `$display` 的格式参数（%b 二进制/%o 八进制/%h 十六进制），不是 `$strobe` 的职责
- 格式统一用 `%d/%b/%o/%h`，三兄弟都支持

### 三兄弟区别（打印时机）
```
$display：立刻打印（执行到就打印当前值）
$monitor：持续监控（信号一变就打印，on/off 控制）
$strobe：沿后打印（本时刻所有赋值完成后的最终值）
```

### $strobe 与 <= 的呼应
```verilog
always @(posedge clk) begin
    a <= a + 1;
    $display("a=%d", a);   // 沿前值（还没更新）
    $strobe("a=%d", a);    // 沿后值（更新完成）
end
```
- `$display` 看到旧值，`$strobe` 看到新值（本时刻最终结果）
- 想"看这一拍最终结果"用 `$strobe`，想"看某时刻值"用 `$display`

**一句话**：display 立刻打印、monitor 持续监控（on/off）、strobe 沿后打印最终值——区别在打印时机，格式统一 %d/%b/%o/%h。
