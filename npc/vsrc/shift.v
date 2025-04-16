module shift(
    input button,
    input rst,
    output reg [7:0] out
);
    reg button_prev = 0; 
    wire feedback = out[4] ^ out[3] ^ out[2] ^ out[0];

    always @(posedge button or posedge rst) begin
        if (rst) begin
            out <= 8'b00000001;
            button_prev <= 0;
        end
        else begin
            // 消抖处理
            if (button && !button_prev) begin 
                out <= {feedback, out[7:1]};
            end
            button_prev <= 0;
        end
    end
endmodule    
