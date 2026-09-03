module seqdet(
    input wire clk,
    input wire rst_n,
    input wire din,          // 串行输入
    output reg detected      // 检测到 1011 输出 1
);

    localparam IDLE = 3'd0;
    localparam S1   = 3'd1;  // 收到 1
    localparam S10  = 3'd2;  // 收到 10
    localparam S101 = 3'd3;  // 收到 101
    localparam S1011= 3'd4;  // 收到 1011（检测成功）

    reg [2:0] state, next_state;

    // 段1：状态跳转（时序）
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            state <= IDLE;
        else
            state <= next_state;
    end
    // 1,0,1,1,0,1,0,1
    // 段2：次态计算（组合）
    always @* begin
        next_state = state;          // 默认保持
        case (state)
            IDLE:  if (din) next_state = S1;
            S1:    if (!din) next_state = S10;
            S10:   if (din) next_state = S101;
            S101:  if (din) next_state = S1011;
                   else next_state = S1;   // 1010 不是，回 S1（重叠）
            S1011: next_state = IDLE;
            default: next_state = IDLE;    // 非法状态防御
        endcase
    end

    // 段3：输出（Moore，只看状态）
    always @* begin
        if (state == S1011)
            detected = 1'b1;
        else
            detected = 1'b0;
    end

endmodule
