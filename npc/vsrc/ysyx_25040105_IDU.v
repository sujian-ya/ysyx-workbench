module ysyx_25040105_IDU (
    input  [31:0]   inst    ,
    output [4:0]    rs1     ,
    output [4:0]    rs2     ,
    output [4:0]    rd      ,
    output [31:0]   imm     ,
    output          reg_wen
);

    // 导入DPI-C函数
    import "DPI-C" function void ebreak_handler();

    // 指令解析逻辑（简化为示例，需根据实际指令集调整）
    wire is_ebreak = (inst == 32'h00100073); // 检测ebreak指令

    // 在时钟边沿或组合逻辑中调用ebreak_handler
    always @(*) begin
        if (is_ebreak) begin
            ebreak_handler(); // 检测到ebreak时调用
        end
    end

    wire [6:0] opcode = inst[6:0];
    wire [2:0] funct3 = inst[14:12];
    wire [6:0] funct7 = inst[31:25];

    assign rs1 = inst[19:15];
    assign rs2 = inst[24:20];
    assign rd = inst[11:7];
    assign imm = {{20{inst[31]}}, inst[31:20]};

    assign reg_wen = (opcode == 7'b0010011) && (funct3 == 3'b000); // ADDI

endmodule