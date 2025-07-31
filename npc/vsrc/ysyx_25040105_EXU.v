module ysyx_25040105_EXU(
    input   [31:0]  pc          , // 程序计数器
    input   [31:0]  rs1_data    , // 源寄存器1的数据
    input   [31:0]  rs2_data    , // 源寄存器2的数据
    input   [31:0]  imm         , // 立即数（来自IDU生成）
    input   [3:0]   alu_op      , // ALU操作控制信号（来自IDU）
    input           alu_src     , // 选择ALU的第二个操作数来源（0:寄存器/1:立即数）
    output  [31:0]  alu_result    // ALU运算结果输出
);

    // ALU操作定义(与IDU中保持一致)
    localparam ALU_ADD          = 4'b0000; // ADD
    localparam ALU_SUB          = 4'b0001; // SUB
    localparam ALU_SLL          = 4'b0010; // SLL
    localparam ALU_SRL          = 4'b0011; // SRL
    localparam ALU_AUIPC        = 4'b0100; // AUIPC指令的特殊处理
    localparam ALU_LUI          = 4'b0101; // LUI指令的特殊处理
    //TODO: 添加更多ALU操作

    // 选择ALU的第二操作数
    wire [31:0] alu_operand2 = alu_src ? imm : rs2_data;

    // ALU运算逻辑 
    reg [31:0] result_reg; // 用于暂存运算结果的寄存器
    always @(*) begin
        case (alu_op)
            ALU_ADD: result_reg = rs1_data + alu_operand2; // ADD
            ALU_SUB: result_reg = rs1_data - alu_operand2; // SUB
            ALU_SLL: result_reg = rs1_data << alu_operand2[4:0]; // SLL
            ALU_SRL: result_reg = rs1_data >> alu_operand2[4:0]; // SRL
            ALU_AUIPC: result_reg = pc + alu_operand2; // AUIPC指令的特殊处理
            ALU_LUI: result_reg = alu_operand2; // LUI指令的特殊处理
            // TODO: 添加更多ALU操作
            default: result_reg = 32'b0; // 默认值，避免综合警告
        endcase
    end

    assign alu_result = result_reg; // 输出ALU结果

endmodule
