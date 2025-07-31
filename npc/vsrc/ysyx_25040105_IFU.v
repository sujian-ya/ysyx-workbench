module ysyx_25040105_IFU (
    input clk           ,
    input rst           ,  
    input jump_en       , // 跳转使能信号
    input [31:0] jump_addr, // 跳转地址
    output reg [31:0] pc
);
    always @(posedge clk) begin
        if (rst) begin
            pc <= 32'h8000_0000;  // 复位值
        end else begin
            // pc <= pc + 4;  // 简单实现，每条指令4字节
            pc <= jump_en ? jump_addr : (pc + 4);
        end
    end
endmodule
