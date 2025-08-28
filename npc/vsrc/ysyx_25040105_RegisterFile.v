module ysyx_25040105_RegisterFile # (
    parameter ADDR_WIDTH = 5,
    DATA_WIDTH = 32
) (
    input                    clk,
    input                    rst,
    input   [ADDR_WIDTH-1:0] raddr1,
    input   [ADDR_WIDTH-1:0] raddr2,
    input   [ADDR_WIDTH-1:0] waddr,
    output  [DATA_WIDTH-1:0] rdata1,
    output  [DATA_WIDTH-1:0] rdata2,
    input   [DATA_WIDTH-1:0] wdata,
    input                    wen,
    output reg [DATA_WIDTH-1:0] rf [0:2**ADDR_WIDTH-1] // 便于查看寄存器波形
);
    // Read logic (combinational)
    assign rdata1 = (raddr1 == 0) ? 32'b0 : rf[raddr1];
    assign rdata2 = (raddr2 == 0) ? 32'b0 : rf[raddr2];

    // 写操作（0号寄存器恒为0）
    always @(posedge clk) begin
        if (rst) begin
            integer i;
            for (i = 0; i < 32; i = i + 1) begin
                rf[i] <= 32'h0;
            end
        end
        begin
            if (wen && waddr != 0) begin
                rf[waddr] <= wdata;
                // rf[waddr] <= (waddr == 0) ? 32'b0 : wdata;
            end
        end
        // $display("wen = %h, waddr = %h, wdata = %h", wen, waddr, wdata);
        // $display("rf[0] = %h, rf[1] = %h, rf[2] = %h", rf[0], rf[1], rf[2]);
    end

endmodule
