module ysyx_25040105_IFU (
    input         clk,
    input         rst,  
    input  [31:0] pc,
    output [31:0] inst
);

    import "DPI-C" function int vaddr_read(input int raddr, input int len);
    assign inst = rst ? 32'h0 : vaddr_read(pc, 32'h4);

endmodule
