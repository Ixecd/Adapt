`timescale 1ns/1ps
module counter_tb;
    reg clk;
    reg rst_n;
    wire [3:0] count;

    counter uut (
        .clk(clk),
        .rst_n(rst_n),
        .count(count)
    );

    initial begin
        clk = 0;
        rst_n = 0;
        #20 rst_n = 1;
        #200 $finish;
    end

    always #5 clk = ~clk;

    initial begin
        $dumpfile("counter.vcd");
        $dumpvars(0, counter_tb);
    end
endmodule
