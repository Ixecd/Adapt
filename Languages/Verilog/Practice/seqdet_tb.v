`timescale 1ns/1ps
module seqdet_tb;
    reg clk;
    reg rst_n;
    reg din;
    wire detected;
    wire [2:0] state;
    wire [2:0] next_state;

    seqdet uut (
        .clk(clk),
        .rst_n(rst_n),
        .din(din),
        .detected(detected)
    );

    assign state = uut.state;   // 引出内部状态观察
    assign next_state = uut.next_state;  // 引出 next_state（组合，先行）

    // 时钟：10ns 周期
    always #5 clk = ~clk;

    // 激励：从 MSB 到 LSB 送 1011_0101（含 1011）
    initial begin
        clk = 0;
        rst_n = 0;
        din = 0;
        #15 rst_n = 1;        // 复位释放

        // 每 10ns 送一位：1,0,1,1,0,1,0,1
        #10 din = 1;  // bit7
        #10 din = 0;  // bit6
        #10 din = 1;  // bit5
        #10 din = 1;  // bit4  ← 这里应检测到 1011
        #10 din = 0;  // bit3
        #10 din = 1;  // bit2
        #10 din = 0;  // bit1
        #10 din = 1;  // bit0
        #20 $finish;
    end

    initial begin
        $dumpfile("seqdet.vcd");
        $dumpvars(0, seqdet_tb);
    end
endmodule
