module ysyx_25040105_soc_top (
    input clk           /* verilator public */,
    input rst           /* verilator public */,
    input [31:0] inst   /* verilator public */,
    output [31:0] pc    /* verilator public */,
    output reg [31:0] rf    [31:0] // 便于查看寄存器波形
);

// IFU
ysyx_25040105_IFU ysyx_25040105_ifu (
    .clk(clk),
    .rst(rst),
    .pc(pc)
);

// IDU
wire [4:0] rs1, rs2, rd;
wire [31:0] imm;
wire reg_wen;
ysyx_25040105_IDU ysyx_25040105_idu (
    .inst(inst),
    .rs1(rs1),
    .rs2(rs2),
    .rd(rd),
    .imm(imm),
    .reg_wen(reg_wen)
);

// Register File
wire [31:0] rs1_data, rs2_data, alu_result;
ysyx_25040105_RegisterFile ysyx_25040105_rf (
    .clk(clk),
    .raddr1(rs1),
    .raddr2(rs2),
    .waddr(rd),
    .rdata1(rs1_data),
    .rdata2(rs2_data),
    .wdata(alu_result),
    .wen(reg_wen),
    .rf(rf)
);

// EXU
ysyx_25040105_EXU ysyx_25040105_exu (
    .rs1_data(rs1_data),
    .rs2_data(rs2_data),
    .imm(imm),
    .alu_result(alu_result)
);

endmodule
