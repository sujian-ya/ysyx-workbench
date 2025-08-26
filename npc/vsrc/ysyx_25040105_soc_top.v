module ysyx_25040105_soc_top (
    input             clk,
    input             rst,
    input      [31:0] inst,
    output     [31:0] pc,
    output reg [31:0] rf [0:31] // 便于查看寄存器波形
);

// 导入DPI-C函数
import "DPI-C" function void sys_exit(int exit_state);
import "DPI-C" function void sim_get_regs(output logic [31:0] rf[0:31]);
// import "DPI-C" function void sim_get_pc(input bit [31:0] rtl_pc[1]);
import "DPI-C" function void sim_get_inst(input bit [31:0] rtl_inst[1]);

// IFU
wire jump_en; // 跳转使能信号
wire [31:0] jump_addr; // 跳转地址
ysyx_25040105_IFU ysyx_25040105_ifu (
    .clk(clk),
    .rst(rst),
    .jump_en(jump_en), // 跳转使能信号
    .jump_addr(jump_addr), // 跳转地址
    .pc(pc)
);

// IDU
wire [4:0] rs1, rs2, rd;
wire [31:0] imm;
wire reg_wen;
wire [3:0] alu_op;
wire alu_src;
ysyx_25040105_IDU ysyx_25040105_idu (
    .inst       (inst),
    .rs1        (rs1),
    .rs2        (rs2),
    .rd         (rd),
    .imm        (imm),
    .reg_wen    (reg_wen),
    .alu_op     (alu_op),
    .alu_src    (alu_src),
    .jump_en    (jump_en) // 跳转使能信号
);

// EXU
wire [31:0] alu_result;
ysyx_25040105_EXU ysyx_25040105_exu (
    .pc         (pc),
    .rs1_data   (rs1_data),
    .rs2_data   (rs2_data),
    .imm        (imm),
    .alu_op     (alu_op),
    .alu_src    (alu_src),
    .alu_result (alu_result),
    .jump_addr  (jump_addr) // 跳转地址
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
wire is_ebreak = (inst == 32'h00100073);
wire a0_state  = (rf[10] == 32'h0); // a0寄存器的值
wire [31:0] exit_state = {31'h0, is_ebreak && a0_state};
// bit [31:0] rtl_pc [1];
bit [31:0] rtl_inst [1];

// integer i;
// 在时钟边沿调用函数，确保数据稳定
always @(posedge clk) begin
    // for (i = 0; i < 32; i = i + 1) begin
    //     $display("Verilog sending reg[%0d]: %h", i, rf[i]);
    // end
    if (!rst) begin
        sim_get_regs(rf);
        // rtl_pc[0] = pc;  // 将pc的值写入数组
        rtl_inst[0] = inst;
        // sim_get_pc(rtl_pc);
        sim_get_inst(rtl_inst);
    end
    if (is_ebreak) begin
        sys_exit(exit_state);
    end
end

endmodule
