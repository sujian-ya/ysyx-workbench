module ps2_keyboard(
	input clk, rst, ps2_clk, ps2_data,
	output [7:0] seg_out [7:0],
	output reg key_valid // 数码管控制信号
);

	reg [9:0] buffer;
	reg [3:0] count;
	reg [2:0] ps2_clk_sync;

	reg [7:0] prev_keycode;
	wire [7:0] ascii;
	reg [7:0] key_count;
	reg [3:0] seg_data [7:0];

	// 同步时序
	always @(posedge clk) begin
		ps2_clk_sync <= {ps2_clk_sync[1:0], ps2_clk};
	end

	/* 记录PS2时钟信号的历史信息，并检测时钟的下降沿，
	*  当发现下降沿时将sampling置一*/
	wire sampling = ps2_clk_sync[2] & ~ps2_clk_sync[1];

	always @(posedge clk) begin
		if (rst == 1) begin
			count <= 0;
			key_valid <= 0;
			key_count <= 0;
			prev_keycode <= 0;
			seg_data <= '{8{4'h0}};
		end
		else begin
			if(sampling) begin
				if(count == 4'd10) begin
					if ((buffer[0] == 0) && // 起始位
						  (ps2_data)       && // 键盘输入数据判断停止位
						  (^buffer[9:1])) begin // 奇校验位
							$display("receive %x", buffer[8:1]);
							if (buffer[8:1] == 8'hf0) begin
								key_valid <= 0;
								// 数码管更新
								key_count <= (key_count < 8'hff) ? key_count + 1 : 8'h0;
							end
							else if (buffer[8:1] != 8'hf0 && prev_keycode != 8'hf0) begin
								key_valid <= 1;
								seg_data[0] <= buffer[4:1];
								seg_data[1] <= buffer[8:5];
								seg_data[2] <= ascii[3:0];
								seg_data[3] <= ascii[7:4];
								seg_data[4] <= key_count[3:0];
								seg_data[5] <= key_count[7:4];
							end
							else begin
								key_valid <= 0;
								seg_data[4] <= key_count[3:0];
								seg_data[5] <= key_count[7:4];
							end
							prev_keycode <= buffer[8:1];
					end
					count <= 0;
				end else begin
					buffer[count] <= ps2_data;
					count <= count + 3'b1;
				end
			end
		end
	end

	//keycode to ascii
	keycode_to_ascii my_keycode_to_ascii(
		.keycode(buffer[8:1]),
		.ascii(ascii)
	);

	seg my_seg0(.in(seg_data[0]), .en(key_valid), .out(seg_out[0]));
	seg my_seg1(.in(seg_data[1]), .en(key_valid), .out(seg_out[1]));
	seg my_seg2(.in(seg_data[2]), .en(key_valid), .out(seg_out[2]));
	seg my_seg3(.in(seg_data[3]), .en(key_valid), .out(seg_out[3]));
	seg my_seg4(.in(seg_data[4]), .en(1), .out(seg_out[4]));
	seg my_seg5(.in(seg_data[5]), .en(1), .out(seg_out[5]));
	seg my_seg6(.in(seg_data[6]), .en(0), .out(seg_out[6]));
	seg my_seg7(.in(seg_data[7]), .en(0), .out(seg_out[7]));

endmodule
