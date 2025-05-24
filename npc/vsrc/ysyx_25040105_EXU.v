module ysyx_25040105_EXU(
    input [31:0] rs1_data,
    input [31:0] rs2_data,
    input [31:0] imm,
    output [31:0] alu_result
);

    assign alu_result = rs1_data + imm;

endmodule