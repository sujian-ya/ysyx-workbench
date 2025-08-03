module ysyx_25040105_soc_top (
    input             clk,
    input             rst,
    input      [31:0] inst,
    output     [31:0] pc,
    output reg [31:0] rf [31:0] // 便于查看寄存器波形
);

// 导入DPI-C函数
import "DPI-C" function void ebreak_handler();
import "DPI-C" function void sys_exit(int a0_state);

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
always @(*) begin
    if (is_ebreak) begin
        ebreak_handler(); // 调用DPI-C函数处理ebreak
        sys_exit(rf[10]);  // 判断a0寄存器是否为0（0表示正常退出）
    end
end

endmodule
