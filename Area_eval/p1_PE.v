`timescale 1ns / 100ps
//`define REG_psum_out

module PE (
  input wire clk,
  input wire rst_n,
  input wire en_store,
  input wire [`p1_act-1:0] in,
  input wire [`p1_psum-1:0] psum_in,
  output reg [`p1_psum-1:0] psum_out
);
  `ifdef REG_psum_out
  reg [`p1_psum-1:0] psum_out_next;
  `endif
  reg [`p1_weight-1:0] weight, weight_next;
  
  always @(posedge clk or negedge rst_n) begin
    if(rst_n == 0) begin
      weight <= 8'd0;
      `ifdef REG_psum_out
      psum_out <= 32'd0;
      `endif
    end
    else begin
      weight <= weight_next;
      `ifdef REG_psum_out
      psum_out <= psum_out_next;
      `endif
      //$display("en_store=%b in=%d weight=%d psum_in=%d psum_out=%d", en_store, $signed(in), $signed(weight), $signed(psum_in), $signed(psum_out));
    end
  end
     
  always @* begin
    if(en_store) begin
      weight_next = in;
      `ifdef REG_psum_out
      psum_out_next = psum_out;
      `else
      psum_out = 32'd0;
      `endif
    end
    else begin
      weight_next = weight;
      `ifdef REG_psum_out
      psum_out_next = {{(`p1_psum-`p1_weight){weight[`p1_weight-1]}}, weight} * {{(`p1_psum-`p1_act){in[`p1_act-1]}}, in} + psum_in;
      `else
      psum_out = {{(`p1_psum-`p1_weight){weight[`p1_weight-1]}}, weight} * {{(`p1_psum-`p1_act){in[`p1_act-1]}}, in} + psum_in;
      `endif
    end
  end

  
endmodule
