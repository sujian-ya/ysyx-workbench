module prior_encoder(
	input [7:0] in,
	input en,
	output reg [2:0] out,
	output reg flag,
	output [7:0] seg_out [7:0]
);

integer i;
always @(*) begin
	out = 3'b0;
	flag = 1'b0;
	for (i = 7; i >= 0; i = i - 1) begin
		if(in[i] == 1) begin
			out = i[2:0];
			flag = 1'b1;
			break;
		end
	end
end

seg my_seg0(.in({1'b0, out}), .en(en), .out(seg_out[0]));
seg my_seg1(.in(4'b0), .en(0), .out(seg_out[1]));
seg my_seg2(.in(4'b0), .en(0), .out(seg_out[2]));
seg my_seg3(.in(4'b0), .en(0), .out(seg_out[3]));
seg my_seg4(.in(4'b0), .en(0), .out(seg_out[4]));
seg my_seg5(.in(4'b0), .en(0), .out(seg_out[5]));
seg my_seg6(.in(4'b0), .en(0), .out(seg_out[6]));
seg my_seg7(.in(4'b0), .en(0), .out(seg_out[7]));

endmodule
