module ysyx_25040105_EXU(
    input   [31:0]  pc,
    input   [31:0]  rs1_data,
    input   [31:0]  rs2_data,
    input   [31:0]  imm,
    input   [7:0]   alu_op,
    output          is_ebreak,
    output  [31:0]  alu_result,
    output  [31:0]  jump_addr
);
    import "DPI-C" function int pmem_read(input int raddr);
    import "DPI-C" function void pmem_write(input int waddr, input int wdata, input byte wmask);

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

    // ---------------- 内部寄存器 ----------------
    reg        is_ebreak_reg;
    reg [31:0] result_reg;
    reg [31:0] jump_addr_reg;
    reg [31:0] read_addr_reg;
    reg [31:0] temp_data;
    reg [7:0]  vmask;

    // ---------------- ALU 运算逻辑 ----------------
    always @(*) begin
        is_ebreak_reg = 1'h0;
        result_reg    = 32'h0;
        jump_addr_reg = pc + 4;
        read_addr_reg = 32'h0;
        temp_data     = 32'h0;
        vmask         = 8'h0;

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

            // Load
            ALU_LB: begin
                read_addr_reg = rs1_data + imm;
                temp_data = pmem_read(read_addr_reg);
                case (read_addr_reg[1:0])
                    2'b00: result_reg = {{24{temp_data[7]}},  temp_data[7:0]};
                    2'b01: result_reg = {{24{temp_data[15]}}, temp_data[15:8]};
                    2'b10: result_reg = {{24{temp_data[23]}}, temp_data[23:16]};
                    2'b11: result_reg = {{24{temp_data[31]}}, temp_data[31:24]};
                endcase
            end
            ALU_LH: begin
                read_addr_reg = rs1_data + imm;
                temp_data = pmem_read(read_addr_reg);
                case (read_addr_reg[1])
                    1'b0: result_reg = {{16{temp_data[15]}}, temp_data[15:0]};
                    1'b1: result_reg = {{16{temp_data[31]}}, temp_data[31:16]};
                endcase
            end
            ALU_LW: begin
                read_addr_reg = rs1_data + imm;
                result_reg    = pmem_read(read_addr_reg);
            end
            ALU_LBU: begin
                read_addr_reg = rs1_data + imm;
                temp_data = pmem_read(read_addr_reg);
                case (read_addr_reg[1:0])
                    2'b00: result_reg = {24'b0, temp_data[7:0]};
                    2'b01: result_reg = {24'b0, temp_data[15:8]};
                    2'b10: result_reg = {24'b0, temp_data[23:16]};
                    2'b11: result_reg = {24'b0, temp_data[31:24]};
                endcase
            end
            ALU_LHU: begin
                read_addr_reg = rs1_data + imm;
                temp_data = pmem_read(read_addr_reg);
                case (read_addr_reg[1])
                    1'b0: result_reg = {16'b0, temp_data[15:0]};
                    1'b1: result_reg = {16'b0, temp_data[31:16]};
                endcase
            end

            // Store
            ALU_SB: begin
                read_addr_reg = rs1_data + imm;  // 临时用作 store_addr
                case (read_addr_reg[1:0])  // offset
                    2'b00: begin
                        vmask = 8'b0001;
                        temp_data = {24'b0, rs2_data[7:0]};  // 无移位
                    end
                    2'b01: begin
                        vmask = 8'b0010;
                        temp_data = {16'b0, rs2_data[7:0], 8'b0};  // 左移 1 字节
                    end
                    2'b10: begin
                        vmask = 8'b0100;
                        temp_data = {8'b0, rs2_data[7:0], 16'b0};  // 左移 2 字节
                    end
                    2'b11: begin
                        vmask = 8'b1000;
                        temp_data = {rs2_data[7:0], 24'b0};  // 左移 3 字节
                    end
                endcase
                pmem_write(read_addr_reg & 32'hfffffffc, temp_data, vmask);  // 用对齐地址
            end

            ALU_SH: begin
                read_addr_reg = rs1_data + imm;
                case (read_addr_reg[1])  // offset for halfword
                    1'b0: begin
                        vmask = 8'b0011;
                        temp_data = {16'b0, rs2_data[15:0]};  // 无移位
                    end
                    1'b1: begin
                        vmask = 8'b1100;
                        temp_data = {rs2_data[15:0], 16'b0};  // 左移 16 位
                    end
                endcase
                pmem_write(read_addr_reg & 32'hfffffffc, temp_data, vmask);  // 对齐地址
            end

            ALU_SW: begin
                read_addr_reg = rs1_data + imm;
                pmem_write(read_addr_reg & 32'hfffffffc, rs2_data, 8'b1111);  // 假设 SW 对齐，或类似处理
            end

            // 系统
            // ALU_ECALL,
            ALU_EBREAK: is_ebreak_reg = 1'h1;

            default: result_reg = 32'h0;
        endcase
    end

    assign alu_result = result_reg;
    assign jump_addr  = jump_addr_reg;
    assign is_ebreak  = is_ebreak_reg;

endmodule
