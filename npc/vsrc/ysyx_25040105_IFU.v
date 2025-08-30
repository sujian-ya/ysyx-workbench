module ysyx_25040105_IFU (
    input clk           ,
    input rst           ,  
    input jump_en       , // 跳转使能信号
    input [31:0] jump_addr, // 跳转地址
    output reg [31:0] pc,
    output reg [31:0] inst
);

    import "DPI-C" function int pmem_read(input int raddr);

    always @(negedge clk) begin
        if (rst) begin
            inst = 32'h0;
        end else begin
        inst = pmem_read(pc);
        $display("pc = %h, inst = %h", pc, inst);
        end
    end

    always @(posedge clk) begin
        if (rst) begin
            pc <= 32'h8000_0000;
        end else begin
            pc <= jump_en ? jump_addr : (pc + 4);
            // inst <= pmem_read(pc);
            $display("pc = %h, inst = %h", pc, inst);
        end
    end
endmodule
