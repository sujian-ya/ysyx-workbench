module ysyx_25040105_EXU(
    input   [31:0]  pc          , // 程序计数器
    input   [31:0]  rs1_data    , // 源寄存器1的数据
    input   [31:0]  rs2_data    , // 源寄存器2的数据
    input   [31:0]  imm         , // 立即数（来自IDU生成）
    input   [4:0]   alu_op      , // ALU操作控制信号（来自IDU）
    output  [31:0]  alu_result  , // ALU运算结果输出
    output  [31:0]  jump_addr     // 跳转地址
);

    import "DPI-C" function int pmem_read(input int raddr);
    import "DPI-C" function void pmem_write(
        input int waddr, input int wdata, input byte wmask);

    // ALU操作定义(与IDU中保持一致)
    localparam ALU_ADD          = 5'h0; // ADD
    localparam ALU_SUB          = 5'h1; // SUB
    localparam ALU_SLL          = 5'h2; // SLL
    localparam ALU_SRL          = 5'h3; // SRL
    localparam ALU_AUIPC        = 5'h4; // AUIPC
    localparam ALU_LUI          = 5'h5; // LUI
    localparam ALU_JAL          = 5'h6; // JAL
    localparam ALU_JALR         = 5'h7; // JALR
    localparam ALU_LW           = 5'h8; // LW
    localparam ALU_SW           = 5'h9; // SW
    localparam ALU_SEQZ         = 5'hA; // SEQZ
    localparam ALU_BEQ          = 5'hB; // BEQ
    localparam ALU_BNE          = 5'hC; // BNE
    localparam ALU_ADDI         = 5'hD; // ADDI
    localparam ALU_SLLI         = 5'hE; // SLLI
    localparam ALU_SRLI         = 5'hF; // SRLI
    //TODO: 添加更多ALU操作

    // ALU运算逻辑 
    reg [31:0] result_reg; // 用于暂存运算结果的寄存器
    reg [31:0] jump_addr_reg;
    reg [31:0] read_addr_reg;

    always @(*) begin
        // 默认初始值，避免综合警告
        result_reg = 32'h0;
        jump_addr_reg = 32'h8000_0000;
        read_addr_reg = 32'h0;
        case (alu_op)
            ALU_ADD: result_reg = rs1_data + rs2_data; // ADD
            ALU_SUB: result_reg = rs1_data - rs2_data; // SUB
            ALU_SLL: result_reg = rs1_data << rs2_data[4:0]; // SLL
            ALU_SRL: result_reg = rs1_data >> rs2_data[4:0]; // SRL
            ALU_ADDI: result_reg = rs1_data + imm; // ADDI
            ALU_SLLI: result_reg = rs1_data << imm[4:0]; // SLLI
            ALU_SRLI: result_reg = rs1_data >> imm[4:0]; // SRLI
            ALU_AUIPC: result_reg = pc + imm; // AUIPC
            ALU_LUI: result_reg = imm; // LUI
            ALU_JAL: begin
                result_reg = pc + 4;
                jump_addr_reg  = pc + imm; // JAL指令的跳转地址
            end
            ALU_JALR: begin
                result_reg = pc + 4; // JALR指令的返回地址
                jump_addr_reg = (rs1_data + imm) & ~32'h1; // JALR跳转地址，结果强制对齐到偶数地址
            end
            ALU_LW: begin
                read_addr_reg = rs1_data + imm; // 计算加载地址
                result_reg = pmem_read(read_addr_reg); // 将计算的地址传递到结果（实际数据加载在MEM阶段处理）
            end
            ALU_SW: begin
                read_addr_reg = rs1_data + imm; // 计算存储地址
                pmem_write(read_addr_reg, rs2_data, 8'b11111111); // 将数据写入内存（实际数据存储在MEM阶段处理）
            end
            ALU_SEQZ: result_reg = (rs1_data == 0) ? 32'h1 : 32'h0; // SEQZ
            ALU_BEQ: begin
                if (rs1_data == rs2_data) begin
                    jump_addr_reg = pc + imm; // BEQ跳转地址
                end else begin
                    jump_addr_reg = pc + 4; // 不跳转，继续执行下一条指令
                end
            end
            ALU_BNE: begin
                if (rs1_data != rs2_data) begin
                    jump_addr_reg = pc + imm; // BNE跳转地址
                end else begin
                    jump_addr_reg = pc + 4; // 不跳转，继续执行下一条指令
                end
            end
            // TODO: 添加更多ALU操作
            default: result_reg = 32'h0; // 默认值，避免综合警告
        endcase
    end

    assign alu_result = result_reg;     // 输出ALU结果
    assign jump_addr = jump_addr_reg;   // 输出跳转地址

endmodule
