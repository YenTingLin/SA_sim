`timescale 1ns / 100ps

module AVG (
  input wire clk,
  input wire rst_n,
  input wire active,
  input wire [`p3_act-1:0] data_in,
  output reg done,
  output reg [`p3_act-1:0] data_out
);
  reg done_next;
  reg [`p3_act-1:0] data_out_next;
  reg [`p3_act+6:0] psum, psum_next, psum_tmp;
  reg [7:0] cnt, cnt_next;
  
  always @(posedge clk or negedge rst_n) begin
    if(rst_n == 0) begin
      done <= 1'd0;
      data_out <= 8'd0;
      psum <= 32'd0;
      cnt <= 8'd0;
    end
    else begin
      done <= done_next;
      data_out <= data_out_next;
      psum <= psum_next;
      cnt <= cnt_next;
      //$display("en_store=%b in=%d weight=%d psum_in=%d psum_out=%d", en_store, $signed(in), $signed(weight), $signed(psum_in), $signed(psum_out));
    end
  end
     
  always @* begin
    psum_tmp = 32'd0;
    if(!active) begin
      done_next = 1'd0;
      data_out_next = data_out;
      psum_next = psum;
      cnt_next = cnt;
    end
    else if(cnt == `p3_cycle - 1) begin
      done_next = 1'd1;
      psum_tmp = psum + {{6{data_in[`p3_act-1]}}, data_in};
      data_out_next = psum_tmp / `p3_cycle;
      psum_next = 32'd0;
      cnt_next = 8'd0;
    end
    else begin
      done_next = 1'd0;
      psum_tmp = psum + {{6{data_in[`p3_act-1]}}, data_in};
      data_out_next = data_out;
      psum_next = psum_tmp;
      cnt_next = cnt + 8'd1;
    end
  end

endmodule
