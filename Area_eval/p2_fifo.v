`timescale 1ns / 100ps

module FIFO (
  input wire clk,
  input wire rst_n,
  input wire en_read,
  input wire en_write,
  input wire [`p2_width-1:0] data_in,
  output reg is_full,
  output reg is_empty,
  output reg [`p2_width-1:0] data_out
);
  reg is_full_next, is_empty_next;
  reg [`p2_width-1:0] data_out_next;
  reg [7:0] cap, cap_tmp, cap_next, head, head_next, tail, tail_next;
  reg [`p2_width-1:0] mem [0:`p2_size-1];
  reg [`p2_width-1:0] mem_next [0:`p2_size-1];
  integer i;
  
  always @(posedge clk or negedge rst_n) begin
    if(rst_n == 0) begin
      is_full <= 1'd0;
      is_empty <= 1'd1;
      data_out <= 0;
      cap <= 8'd0;
      head <= 8'd0;
      tail <= 8'd0;
      for(i = 0; i < `p2_size; i=i+1) begin
        mem[i] <= 0;
      end
    end
    else begin
      is_full <= is_full_next;
      is_empty <= is_empty_next;
      data_out <= data_out_next;
      cap <= cap_next;
      head <= head_next;
      tail <= tail_next;
      for(i = 0; i < `p2_size; i=i+1) begin
        mem[i] <= mem_next[i];
      end
      //$display("en_store=%b in=%d weight=%d psum_in=%d psum_out=%d", en_store, $signed(in), $signed(weight), $signed(psum_in), $signed(psum_out));
    end
  end
     
  always @* begin
    is_full_next = is_full;
    is_empty_next = is_empty;
    data_out_next = data_out;
    head_next = head;
    tail_next = tail;
    for(i = 0; i < `p2_size; i=i+1) begin
      mem_next[i] = mem[i];
    end
    if(en_read) begin
      if(cap > 0) begin
        data_out_next = mem[head];
        if(head == `p2_size - 1)
          head_next = 8'd0;
        else
          head_next = head + 8'd1;
        cap_tmp = cap - 8'd1;
        is_empty_next = 1'd0;
      end
      else begin
        cap_tmp = cap;
        is_empty_next = 1'd1;
      end
    end
    else begin
      cap_tmp = cap;
    end
    if(en_write) begin
      if(cap < `p2_size) begin
        mem_next[tail] = data_in;
        if(tail == `p2_size - 1)
          tail_next = 8'd0;
        else
          tail_next = tail + 8'd1;
        cap_next = cap_tmp + 8'd1;
        is_full_next = 1'd0;
      end
      else begin
        cap_next = cap_tmp;
        is_full_next = 1'd1;
      end
    end
    else begin
      cap_next = cap_tmp;
    end
  end

  
endmodule
