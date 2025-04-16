module shift_seg (
	input rst,          // 复位信号
	input button,       // 按钮信号
	output [7:0] seg_out [7:0]
);
	reg [7:0] out;   // 8位移位寄存器输出

// 实例化移位寄存器模块
shift my_shift (.button(button), .rst(rst), .out(out));

// 实例化数码管显示模块（低4位）
seg my_seg0 (
		.in(out[3:0]),   // 低4位输入
		.en(1),					 // 使能信号
		.out(seg_out[0]) // 数码管0输出
);

// 实例化数码管显示模块（高4位）
seg my_seg1 (
		.in(out[7:4]),   // 高4位输入
		.en(1),          // 使能信号
		.out(seg_out[1]) // 数码管1输出
);

seg my_seg2(.in(4'h0), .en(0), .out(seg_out[2]));
seg my_seg3(.in(4'h0), .en(0), .out(seg_out[3]));
seg my_seg4(.in(4'h0), .en(0), .out(seg_out[4]));
seg my_seg5(.in(4'h0), .en(0), .out(seg_out[5]));
seg my_seg6(.in(4'h0), .en(0), .out(seg_out[6]));
seg my_seg7(.in(4'h0), .en(0), .out(seg_out[7]));
endmodule
