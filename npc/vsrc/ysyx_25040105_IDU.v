module ysyx_25040105_IDU (
    input  [31:0]   inst    ,   // 32位输入指令
    output [4:0]    rs1     ,   // 源寄存器1地址
    output [4:0]    rs2     ,   // 源寄存器2地址
    output [4:0]    rd      ,   // 目的寄存器地址
    output [31:0]   imm     ,   // 生成的立即数
    output          reg_wen ,   // 寄存器写使能信号
    output [4:0]    alu_op  ,   // ALU操作控制信号
    output          jump_en     // 跳转使能信号
);

    // 指令 opcode 定义
    localparam OPCODE_LOAD      = 7'b0000011;
    localparam OPCODE_OP_IMM    = 7'b0010011;
    localparam OPCODE_STORE     = 7'b0100011;
    localparam OPCODE_OP        = 7'b0110011;
    localparam OPCODE_BRANCH    = 7'b1100011;
    localparam OPCODE_JALR      = 7'b1100111;
    localparam OPCODE_JAL       = 7'b1101111;
    localparam OPCODE_AUIPC     = 7'b0010111;
    localparam OPCODE_LUI       = 7'b0110111;
    localparam OPCODE_SYSTEM    = 7'b1110011;

    // 功能码定义
    localparam FUNCT3_ADDI      = 3'b000;
    localparam FUNCT3_ADD       = 3'b000;
    localparam FUNCT3_SUB       = 3'b000;
    localparam FUNCT3_SLL       = 3'b001;
    localparam FUNCT3_SRL       = 3'b101;
    localparam FUNCT3_SLLI      = 3'b001;
    localparam FUNCT3_SRLI      = 3'b101;
    localparam FUNCT3_LW        = 3'b010;
    localparam FUNCT3_SW        = 3'b010;
    localparam FUNCT3_SEQZ      = 3'b011;
    localparam FUNCT3_BEQ       = 3'b000;
    localparam FUNCT3_BNE       = 3'b001;

    // ALU操作定义(与EXU中保持一致)
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

    // 指令字段提取
    wire [6:0] opcode = inst[6:0];      // 提取操作码(7位)
    wire [2:0] funct3 = inst[14:12];    // 提取funct3功能码(3位)
    wire [6:0] funct7 = inst[31:25];    // 提取funct7功能码(7位)
    // wire [4:0] shamt  = inst[24:20];    // 提取移位量(5位)

    // 寄存器号提取
    assign rs1 = inst[19:15];           // 源寄存器1地址(5位)
    assign rs2 = inst[24:20];           // 源寄存器2地址(5位)
    assign rd = inst[11:7];            // 目的寄存器地址(5位)

    // 立即数生成逻辑
    reg [31:0] imm_reg;
    always @(*) begin
        case (opcode)
            // I型立即数(用于立即数运算、加载、JALR等)
            OPCODE_OP_IMM,
            OPCODE_LOAD,
            OPCODE_JALR:
                imm_reg = {{20{inst[31]}}, inst[31:20]}; // I-type

            // S型立即数(用于存储指令)
            OPCODE_STORE:
                imm_reg = {{20{inst[31]}}, inst[31:25], inst[11:7]}; // S-type

            // B型立即数(用于分支指令)
            OPCODE_BRANCH:
                imm_reg = {{19{inst[31]}}, inst[31], inst[7], inst[30:25], inst[11:8], 1'b0}; // B-type

            // J型立即数(用于JAL指令)
            OPCODE_JAL:
                imm_reg = {{11{inst[31]}}, inst[31], inst[19:12], inst[20], inst[30:21], 1'b0}; // J-type

            // U型立即数(用于LUI和AUIPC指令)
            OPCODE_LUI,
            OPCODE_AUIPC:
                imm_reg = {inst[31:12], 12'b0}; // U-type

            default:
                imm_reg = 32'b0; // 默认立即数为0
        endcase
    end

    // ALU控制信号生成
    reg [4:0] alu_op_reg;       // ALU操作码寄存器
    reg reg_wen_reg;            // 寄存器写使能寄存器
    // 跳转使能信号和跳转地址初始化
    assign jump_en = (opcode == OPCODE_JAL || 
        opcode == OPCODE_JALR ||
        opcode == OPCODE_BRANCH
    ); 

    always @(*) begin
        // 默认值
        alu_op_reg  = 5'h0;
        reg_wen_reg = 1'b0; // 默认不写寄存器

        case (opcode)
            // 立即数运算指令(如ADDI、SLLI等)
            OPCODE_OP_IMM: begin
                reg_wen_reg = 1'b1; // 写目的寄存器
                case (funct3)
                    FUNCT3_ADDI:    alu_op_reg = ALU_ADDI;   // ADDI
                    FUNCT3_SLLI:    alu_op_reg = ALU_SLLI;   // SLLI
                    FUNCT3_SRLI:    alu_op_reg = ALU_SRLI;   // SRLI
                    FUNCT3_SEQZ:    alu_op_reg = ALU_SEQZ;  // SEQZ
                    // TODO:
                    default: alu_op_reg = 5'hx;
                endcase
            end

            // 寄存器运算指令(如ADD、SUB等)
            OPCODE_OP: begin
                reg_wen_reg = 1'b1; // 写目的寄存器
                case (funct3)
                    FUNCT3_ADD: begin
                        if (funct7[5])
                            alu_op_reg = ALU_SUB; // SUB
                        else
                            alu_op_reg = ALU_ADD; // ADD
                    end
                    FUNCT3_SLL:     alu_op_reg = ALU_SLL;   // SLL
                    FUNCT3_SRL:     alu_op_reg = ALU_SRL;   //
                    // TODO:
                    default: alu_op_reg = 5'hx;
                endcase
            end

            // 跳转指令(如JALR)
            OPCODE_JALR: begin
                reg_wen_reg = 1'b1; // 写目的寄存器(存储返回地址)
                alu_op_reg = ALU_JALR; // JALR指令的特殊处理
            end

            // 跳转指令(如JAL)
            OPCODE_JAL: begin
                reg_wen_reg = 1'b1; // 写目的寄存器(存储返回地址)
                alu_op_reg = ALU_JAL; // JAL指令的特殊处理
            end

            // 加载指令(如LW、LB等)
            OPCODE_LOAD: begin
                reg_wen_reg = 1'b1; // 写目的寄存器(存储加载的数据)
                case (funct3)
                    FUNCT3_LW: alu_op_reg = ALU_LW; // 地址计算(基地址+偏移量)
                    default: alu_op_reg = 5'hx;
                endcase
            end

            // 储存指令（如SW、SB等）
            OPCODE_STORE: begin
                reg_wen_reg = 1'b0; // 不写寄存器
                case (funct3)
                    FUNCT3_SW: alu_op_reg = ALU_SW; // 地址计算(基地址+偏移量)
                    default: alu_op_reg = 5'hx;
                endcase
            end

            // 分支指令（如BEQ、BNE等）
            OPCODE_BRANCH: begin
                reg_wen_reg = 1'b0; // 不写寄存器
                case (funct3) 
                    FUNCT3_BEQ: alu_op_reg = ALU_BEQ; // BEQ
                    FUNCT3_BNE: alu_op_reg = ALU_BNE; // BNE
                    default: alu_op_reg = 5'hx;
                endcase
            end

            OPCODE_AUIPC: begin
                reg_wen_reg = 1'b1; // 写目的寄存器(存储PC+立即数)
                alu_op_reg = ALU_AUIPC; // AUIPC指令特殊处理
            end

            OPCODE_LUI: begin
                reg_wen_reg = 1'b1; // 写目的寄存器(存储立即数)
                alu_op_reg = ALU_LUI; // LUI指令特殊处理
            end

            // TODO:
            default: begin
                // 其他指令或未定义指令
            end
        endcase
    end

    assign imm     = imm_reg;       // 输出立即数
    assign alu_op  = alu_op_reg;    // 输出ALU操作码
    assign reg_wen = reg_wen_reg;   // 输出寄存器写使能信号

endmodule
