module ALU (
    input [3:0] A,
    input [3:0] B,
    input [2:0] F,
    output reg [3:0] Out,
    output reg Zero,
    output reg Overflow,
    output reg Carry
);

    always @(*) begin
        case (F)
            3'b000: begin // 加法
                {Carry, Out} = A + B;
                Overflow = (A[3] == B[3]) && (Out[3] != A[3]);
            end
            3'b001: begin // 减法
                {Carry, Out} = A - B;
                Overflow = (A[3] != B[3]) && (Out[3] != A[3]);
            end
            3'b010: begin // 取反
                Out = ~A;
                Carry = 0;
                Overflow = 0;
            end
            3'b011: begin // 与
                Out = A & B;
                Carry = 0;
                Overflow = 0;
            end
            3'b100: begin // 或
                Out = A | B;
                Carry = 0;
                Overflow = 0;
            end
            3'b101: begin // 异或
                Out = A ^ B;
                Carry = 0;
                Overflow = 0;
            end
            3'b110: begin // 比较大小
								if (A[3] == B[3]) begin
									if (A > B) Out = 4'b0000;
									else Out = 4'b0001;
								end
								else if (A[3] == 1 && B[3] == 0) Out = 4'b0001;
								else Out = 4'b0000;
                //Out = (A < B) ? 4'b0001 : 4'b0000;
                Carry = 0;
                Overflow = 0;
            end
            3'b111: begin // 判断相等
                Out = (A == B) ? 4'b0001 : 4'b0000;
                Carry = 0;
                Overflow = 0;
            end
            default: begin
                Out = 4'b0000;
                Carry = 0;
                Overflow = 0;
            end
        endcase

        Zero = (Out == 4'b0000);
    end

endmodule
