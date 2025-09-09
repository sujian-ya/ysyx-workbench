module ysyx_25040105_IDU (
    input  [31:0]   inst    ,   // 32位输入指令
    output [4:0]    rs1     ,   // 源寄存器1地址
    output [4:0]    rs2     ,   // 源寄存器2地址
    output [4:0]    rd      ,   // 目的寄存器地址
    output [31:0]   imm     ,   // 生成的立即数
    output          reg_wen ,   // 寄存器写使能信号
    output [7:0]    alu_op  ,   // ALU操作控制信号
    output          jump_en     // 跳转使能信号
);

    // ---------------- opcode 定义 ----------------
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

    // ---------------- funct3 定义 ----------------
    localparam FUNCT3_ADD_SUB   = 3'b000;
    localparam FUNCT3_SLL       = 3'b001;
    localparam FUNCT3_SLT       = 3'b010;
    localparam FUNCT3_SLTU      = 3'b011;
    localparam FUNCT3_XOR       = 3'b100;
    localparam FUNCT3_SRL_SRA   = 3'b101;
    localparam FUNCT3_OR        = 3'b110;
    localparam FUNCT3_AND       = 3'b111;

    localparam FUNCT3_BEQ       = 3'b000;
    localparam FUNCT3_BNE       = 3'b001;
    localparam FUNCT3_BLT       = 3'b100;
    localparam FUNCT3_BGE       = 3'b101;
    localparam FUNCT3_BLTU      = 3'b110;
    localparam FUNCT3_BGEU      = 3'b111;

    localparam FUNCT3_LB        = 3'b000;
    localparam FUNCT3_LH        = 3'b001;
    localparam FUNCT3_LW        = 3'b010;
    localparam FUNCT3_LBU       = 3'b100;
    localparam FUNCT3_LHU       = 3'b101;

    localparam FUNCT3_SB        = 3'b000;
    localparam FUNCT3_SH        = 3'b001;
    localparam FUNCT3_SW        = 3'b010;

    // ---------------- ALU操作码(与EXU保持一致) ----------------
    // 算术逻辑
    localparam ALU_ADD   = 8'h00;
    localparam ALU_SUB   = 8'h01;
    localparam ALU_XOR   = 8'h02;
    localparam ALU_OR    = 8'h03;
    localparam ALU_AND   = 8'h04;
    localparam ALU_ADDI  = 8'h05;
    localparam ALU_XORI  = 8'h06;
    localparam ALU_ORI   = 8'h07;
    localparam ALU_ANDI  = 8'h08;

    // 移位
    localparam ALU_SLL   = 8'h09;
    localparam ALU_SRL   = 8'h0A;
    localparam ALU_SRA   = 8'h0B;
    localparam ALU_SLLI  = 8'h0C;
    localparam ALU_SRLI  = 8'h0D;
    localparam ALU_SRAI  = 8'h0E;

    // 比较
    localparam ALU_SLT   = 8'h0F;
    localparam ALU_SLTU  = 8'h10;
    localparam ALU_SLTI  = 8'h11;
    localparam ALU_SLTIU = 8'h12;

    // 立即数/高位操作
    localparam ALU_LUI   = 8'h13;
    localparam ALU_AUIPC = 8'h14;

    // 跳转/分支
    localparam ALU_JAL   = 8'h15;
    localparam ALU_JALR  = 8'h16;
    localparam ALU_BEQ   = 8'h17;
    localparam ALU_BNE   = 8'h18;
    localparam ALU_BLT   = 8'h19;
    localparam ALU_BGE   = 8'h1A;
    localparam ALU_BLTU  = 8'h1B;
    localparam ALU_BGEU  = 8'h1C;

    // Load
    localparam ALU_LB    = 8'h1D;
    localparam ALU_LH    = 8'h1E;
    localparam ALU_LW    = 8'h1F;
    localparam ALU_LBU   = 8'h20;
    localparam ALU_LHU   = 8'h21;

    // Store
    localparam ALU_SB    = 8'h22;
    localparam ALU_SH    = 8'h23;
    localparam ALU_SW    = 8'h24;

    // 系统
    localparam ALU_ECALL  = 8'h25;
    localparam ALU_EBREAK = 8'h26;

    // ---------------- 指令字段提取 ----------------
    wire [6:0] opcode = inst[6:0];
    wire [2:0] funct3 = inst[14:12];
    wire [6:0] funct7 = inst[31:25];

    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];
    assign rd  = inst[11:7];

    // ---------------- 立即数生成 ----------------
    reg [31:0] imm_reg;
    always @(*) begin
        case (opcode)
            OPCODE_OP_IMM,
            OPCODE_LOAD,
            OPCODE_JALR:
                imm_reg = {{20{inst[31]}}, inst[31:20]}; // I-type

            OPCODE_STORE:
                imm_reg = {{20{inst[31]}}, inst[31:25], inst[11:7]}; // S-type

            OPCODE_BRANCH:
                imm_reg = {{19{inst[31]}}, inst[31], inst[7], inst[30:25], inst[11:8], 1'b0}; // B-type

            OPCODE_JAL:
                imm_reg = {{11{inst[31]}}, inst[31], inst[19:12], inst[20], inst[30:21], 1'b0}; // J-type

            OPCODE_LUI,
            OPCODE_AUIPC:
                imm_reg = {inst[31:12], 12'b0}; // U-type

            default:
                imm_reg = 32'b0;
        endcase
    end

    // ---------------- ALU控制逻辑 ----------------
    reg [7:0] alu_op_reg;
    reg reg_wen_reg;

    // ---------------- 跳转使能信号 ----------------
    assign jump_en = (opcode == OPCODE_JAL || 
                      opcode == OPCODE_JALR ||
                      opcode == OPCODE_BRANCH);

    always @(*) begin
        alu_op_reg  = 8'h0;
        reg_wen_reg = 1'b0;

        case (opcode)
            // ---------------- R型指令 ----------------
            OPCODE_OP: begin
                reg_wen_reg = 1'b1;
                case (funct3)
                    FUNCT3_ADD_SUB: alu_op_reg = (funct7[5]) ? ALU_SUB : ALU_ADD;
                    FUNCT3_SLL:     alu_op_reg = ALU_SLL;
                    FUNCT3_SLT:     alu_op_reg = ALU_SLT;
                    FUNCT3_SLTU:    alu_op_reg = ALU_SLTU;
                    FUNCT3_XOR:     alu_op_reg = ALU_XOR;
                    FUNCT3_SRL_SRA: alu_op_reg = (funct7[5]) ? ALU_SRA : ALU_SRL;
                    FUNCT3_OR:      alu_op_reg = ALU_OR;
                    FUNCT3_AND:     alu_op_reg = ALU_AND;
                    default:        alu_op_reg = 8'hx;
                endcase
            end

            // ---------------- I型运算 ----------------
            OPCODE_OP_IMM: begin
                reg_wen_reg = 1'b1;
                case (funct3)
                    FUNCT3_ADD_SUB: alu_op_reg = ALU_ADDI;
                    FUNCT3_SLL:     alu_op_reg = ALU_SLLI;
                    FUNCT3_SLT:     alu_op_reg = ALU_SLTI;
                    FUNCT3_SLTU:    alu_op_reg = ALU_SLTIU;
                    FUNCT3_XOR:     alu_op_reg = ALU_XORI;
                    FUNCT3_SRL_SRA: alu_op_reg = (funct7[5]) ? ALU_SRAI : ALU_SRLI;
                    FUNCT3_OR:      alu_op_reg = ALU_ORI;
                    FUNCT3_AND:     alu_op_reg = ALU_ANDI;
                    default:        alu_op_reg = 8'hx;
                endcase
            end

            // ---------------- Load ----------------
            OPCODE_LOAD: begin
                reg_wen_reg = 1'b1;
                case (funct3)
                    FUNCT3_LB:  alu_op_reg = ALU_LB;
                    FUNCT3_LH:  alu_op_reg = ALU_LH;
                    FUNCT3_LW:  alu_op_reg = ALU_LW;
                    FUNCT3_LBU: alu_op_reg = ALU_LBU;
                    FUNCT3_LHU: alu_op_reg = ALU_LHU;
                    default:    alu_op_reg = 8'hx;
                endcase
            end

            // ---------------- Store ----------------
            OPCODE_STORE: begin
                reg_wen_reg = 1'b0;
                case (funct3)
                    FUNCT3_SB: alu_op_reg = ALU_SB;
                    FUNCT3_SH: alu_op_reg = ALU_SH;
                    FUNCT3_SW: alu_op_reg = ALU_SW;
                    default:   alu_op_reg = 8'hx;
                endcase
            end

            // ---------------- Branch ----------------
            OPCODE_BRANCH: begin
                reg_wen_reg = 1'b0;
                case (funct3)
                    FUNCT3_BEQ:  alu_op_reg = ALU_BEQ;
                    FUNCT3_BNE:  alu_op_reg = ALU_BNE;
                    FUNCT3_BLT:  alu_op_reg = ALU_BLT;
                    FUNCT3_BGE:  alu_op_reg = ALU_BGE;
                    FUNCT3_BLTU: alu_op_reg = ALU_BLTU;
                    FUNCT3_BGEU: alu_op_reg = ALU_BGEU;
                    default:     alu_op_reg = 8'hx;
                endcase
            end

            // ---------------- 跳转 ----------------
            OPCODE_JAL: begin
                reg_wen_reg = 1'b1;
                alu_op_reg  = ALU_JAL;
            end
            OPCODE_JALR: begin
                reg_wen_reg = 1'b1;
                alu_op_reg  = ALU_JALR;
            end

            // ---------------- U型 ----------------
            OPCODE_LUI: begin
                reg_wen_reg = 1'b1;
                alu_op_reg  = ALU_LUI;
            end
            OPCODE_AUIPC: begin
                reg_wen_reg = 1'b1;
                alu_op_reg  = ALU_AUIPC;
            end

            // ---------------- SYSTEM ----------------
            OPCODE_SYSTEM: begin
                case (inst[31:20]) // 根据 funct12 判断
                    12'h000: begin alu_op_reg = ALU_ECALL;  reg_wen_reg = 1'b0; end
                    12'h001: begin alu_op_reg = ALU_EBREAK; reg_wen_reg = 1'b0; end
                    default: alu_op_reg = 8'hx;
                endcase
            end

            default: begin
                alu_op_reg  = 8'hx;
                reg_wen_reg = 1'b0;
            end
        endcase
    end

    assign imm     = imm_reg;
    assign alu_op  = alu_op_reg;
    assign reg_wen = reg_wen_reg;

endmodule
