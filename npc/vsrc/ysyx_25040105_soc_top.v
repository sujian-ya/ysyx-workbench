module ysyx_25040105_soc_top (
    input             clk,
    input             rst,
    output     [31:0] inst,
    output reg [31:0] pc,
    output reg [31:0] rf [0:31] // 便于查看寄存器波形
);

initial begin
    pc = 32'h8000_0000;
end

// 导入DPI-C函数
import "DPI-C" function void sys_exit(int exit_state);
import "DPI-C" function void sim_get_inst(input bit [31:0] rtl_inst[1]);
import "DPI-C" function int vaddr_read(input int raddr, input int len);
import "DPI-C" function void vaddr_write(input int addr, input int len, input int data);

// IFU
wire jump_en; // 跳转使能信号
wire [31:0] jump_addr; // 跳转地址
ysyx_25040105_IFU ysyx_25040105_ifu (
    .clk(clk),
    .rst(rst),
    .pc(pc),
    .inst(inst)
);

// IDU
wire [4:0] rs1, rs2, rd;
wire [31:0] imm;
wire reg_wen;
wire [7:0] alu_op;
ysyx_25040105_IDU ysyx_25040105_idu (
    .inst       (inst),
    .rs1        (rs1),
    .rs2        (rs2),
    .rd         (rd),
    .imm        (imm),
    .reg_wen    (reg_wen),
    .alu_op     (alu_op),
    .jump_en    (jump_en), // 跳转使能信号
    .mem_wen    (mem_wen)
);

// EXU
wire [31:0] alu_result;
wire        mem_wen;
wire [31:0] mem_addr;
wire [31:0] mem_len;
wire [31:0] mem_data;
ysyx_25040105_EXU ysyx_25040105_exu (
    .pc         (pc),
    .rs1_data   (rs1_data),
    .rs2_data   (rs2_data),
    .imm        (imm),
    .alu_op     (alu_op),
    .is_ebreak  (is_ebreak),
    .alu_result (alu_result),
    .jump_addr  (jump_addr), // 跳转地址

    .mem_addr   (mem_addr),
    .mem_len    (mem_len),
    .mem_data   (mem_data)
);

// Register File
wire [31:0] rs1_data, rs2_data;
ysyx_25040105_RegisterFile ysyx_25040105_rf (
    .clk        (clk),
    .rst        (rst),
    .raddr1     (rs1),
    .raddr2     (rs2),
    .waddr      (rd),
    .rdata1     (rs1_data),
    .rdata2     (rs2_data),
    .wdata      (alu_result),
    .wen        (reg_wen),
    .rf         (rf)
);

// 检测ebreak指令
wire is_ebreak;
wire a0_state  = (rf[10] == 32'h0); // a0寄存器的值
wire [31:0] exit_state = {31'h0, is_ebreak && a0_state};
bit [31:0] rtl_inst [1];

// 时钟上升沿读取指令，执行ebreak，更新pc，更新内存（防止多次写入导致串口异常）
always @(posedge clk) begin
    rtl_inst[0] = inst;
    sim_get_inst(rtl_inst);
    if (is_ebreak) begin
        sys_exit(exit_state);
    end

    if (rst) begin
        pc <= 32'h8000_0000;
    end else begin
        if (jump_en) begin
            pc <= jump_addr;
        end else begin
            pc <= pc + 4;
        end
    end

    if (mem_wen) begin
        vaddr_write(mem_addr, mem_len, mem_data);
    end
end

endmodule
