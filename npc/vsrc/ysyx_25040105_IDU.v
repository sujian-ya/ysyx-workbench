module ysyx_25040105_IDU (
    input  [31:0]   inst    ,
    output [4:0]    rs1     ,
    output [4:0]    rs2     ,
    output [4:0]    rd      ,
    output [31:0]   imm     ,
    output          reg_wen
);
    wire [6:0] opcode = inst[6:0];
    wire [2:0] funct3 = inst[14:12];
    wire [6:0] funct7 = inst[31:25];

    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];
    assign rd = inst[11:7];
    assign imm = {{20{inst[31]}}, inst[31:20]};

    assign reg_wen = (opcode == 7'b0010011) && (funct3 == 3'b000); // ADDI

endmodule