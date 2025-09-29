module ysyx_25040105_EXU(
    input   [31:0]  pc,
    input   [31:0]  rs1_data,
    input   [31:0]  rs2_data,
    input   [31:0]  imm,
    input   [7:0]   alu_op,
    output          is_ebreak,
    output  [31:0]  alu_result,
    output  [31:0]  jump_addr,
    output  [31:0]  mem_addr,
    output  [31:0]  mem_len,
    output  [31:0]  mem_data
);
    // ---------------- DPI-C 函数声明 ----------------
    import "DPI-C" function int vaddr_read(input int raddr, input int len);
    import "DPI-C" function void vaddr_write(input int addr, input int len, input int data);
    import "DPI-C" function int csr_read(input int csr_addr);
    import "DPI-C" function void csr_write(input int csr_addr, input int data);
    import "DPI-C" function int isa_raise_intr(input int NO, input int epc);
    import "DPI-C" function int isa_ret_intr();

    // ---------------- ALU op 定义 ----------------
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
    localparam ALU_CSRRW  = 8'h27;
    localparam ALU_CSRRS  = 8'h28;
    localparam ALU_MRET   = 8'h29;

    // ---------------- 内部寄存器 ----------------
    reg        is_ebreak_reg;
    reg [31:0] result_reg;
    reg [31:0] temp_data;
    reg [31:0] jump_addr_reg;
    reg [31:0]  mem_addr_reg;
    reg [31:0]   mem_len_reg;
    reg [31:0]  mem_data_reg;
    
    // ---------------- ALU 运算逻辑 ----------------
    always @(*) begin
        is_ebreak_reg = 1'h0;
        result_reg    = 32'h0;
        temp_data     = 32'h0;
        jump_addr_reg = pc + 4;
        mem_addr_reg  = 32'h8000_0000;
        mem_len_reg   = 32'h1;
        mem_data_reg  = 32'h0;
        

        case (alu_op)
            // 算术逻辑
            ALU_ADD:   result_reg = rs1_data + rs2_data;
            ALU_SUB:   result_reg = rs1_data - rs2_data;
            ALU_XOR:   result_reg = rs1_data ^ rs2_data;
            ALU_OR:    result_reg = rs1_data | rs2_data;
            ALU_AND:   result_reg = rs1_data & rs2_data;
            ALU_ADDI:  result_reg = rs1_data + imm;
            ALU_XORI:  result_reg = rs1_data ^ imm;
            ALU_ORI:   result_reg = rs1_data | imm;
            ALU_ANDI:  result_reg = rs1_data & imm;

            // 移位
            ALU_SLL:   result_reg = rs1_data << rs2_data[4:0];
            ALU_SRL:   result_reg = rs1_data >> rs2_data[4:0];
            ALU_SRA:   result_reg = $signed(rs1_data) >>> rs2_data[4:0];
            ALU_SLLI:  result_reg = rs1_data << imm[4:0];
            ALU_SRLI:  result_reg = rs1_data >> imm[4:0];
            ALU_SRAI:  result_reg = $signed(rs1_data) >>> imm[4:0];

            // 比较
            ALU_SLT:   result_reg = ($signed(rs1_data) < $signed(rs2_data)) ? 32'h1 : 32'h0;
            ALU_SLTU:  result_reg = (rs1_data < rs2_data) ? 32'h1 : 32'h0;
            ALU_SLTI:  result_reg = ($signed(rs1_data) < $signed(imm)) ? 32'h1 : 32'h0;
            ALU_SLTIU: result_reg = (rs1_data < imm) ? 32'h1 : 32'h0;

            // 高位立即数
            ALU_LUI:   result_reg = imm;
            ALU_AUIPC: result_reg = pc + imm;

            // 跳转
            ALU_JAL: begin
                result_reg    = pc + 4;
                jump_addr_reg = pc + imm;
            end
            ALU_JALR: begin
                result_reg    = pc + 4;
                jump_addr_reg = (rs1_data + imm) & ~32'h1;
            end

            // 分支
            ALU_BEQ:   if (rs1_data == rs2_data) jump_addr_reg = pc + imm;
            ALU_BNE:   if (rs1_data != rs2_data) jump_addr_reg = pc + imm;
            ALU_BLT:   if ($signed(rs1_data) <  $signed(rs2_data)) jump_addr_reg = pc + imm;
            ALU_BGE:   if ($signed(rs1_data) >= $signed(rs2_data)) jump_addr_reg = pc + imm;
            ALU_BLTU:  if (rs1_data < rs2_data) jump_addr_reg = pc + imm;
            ALU_BGEU:  if (rs1_data >= rs2_data) jump_addr_reg = pc + imm;

            // Load（lb和lh手动进行拓展）
            ALU_LB:  begin
                temp_data = vaddr_read(rs1_data + imm, 1);
                result_reg = {{24{temp_data[7]}}, temp_data[7:0]};
            end
            ALU_LH:  begin
                temp_data = vaddr_read(rs1_data + imm, 2);
                result_reg = {{16{temp_data[15]}}, temp_data[15:0]};
            end
            ALU_LW:  result_reg = vaddr_read(rs1_data + imm, 4);
            ALU_LBU: result_reg = vaddr_read(rs1_data + imm, 1);
            ALU_LHU: result_reg = vaddr_read(rs1_data + imm, 2);

            // Store
            ALU_SB: begin
                mem_addr_reg = rs1_data + imm;
                mem_len_reg = 1;
                mem_data_reg = rs2_data;
            end
            ALU_SH: begin
                mem_addr_reg = rs1_data + imm;
                mem_len_reg = 2;
                mem_data_reg = rs2_data;
            end
            ALU_SW: begin
                mem_addr_reg = rs1_data + imm;
                mem_len_reg = 4;
                mem_data_reg = rs2_data;
            end

            // 系统
            ALU_EBREAK: is_ebreak_reg = 1'h1;
            ALU_ECALL: jump_addr_reg  = isa_raise_intr(32'hB, pc);
            ALU_MRET: jump_addr_reg = isa_ret_intr();
            ALU_CSRRW: begin
                result_reg = csr_read(imm);
                csr_write(imm, rs1_data);
            end
            ALU_CSRRS:  begin
                result_reg = csr_read(imm);
                if (rs1_data != 32'h0) csr_write(imm, result_reg | rs1_data);
            end
            default: result_reg = 32'h0;
        endcase
    end

    assign alu_result = result_reg;
    assign jump_addr  = jump_addr_reg;
    assign is_ebreak  = is_ebreak_reg;
    assign mem_addr   = mem_addr_reg;
    assign mem_len    = mem_len_reg;
    assign mem_data   = mem_data_reg;

endmodule
