`timescale 1ns / 100ps

module ACT (
  input wire clk,
  input wire rst_n,
  input wire [1:0] op_type,
  input wire [31:0] data_in,
  output reg [`p4_act-1:0] data_act
);
  reg [`p4_act-1:0] data_act_next;
  reg signed [`p4_bias-1:0] b_f, b_f_next;
  reg [9:0] M_w_m, M_w_m_next;
  reg signed [5:0] M_w_e, M_w_e_next;
  reg [9:0] M_a_m, M_a_m_next;
  reg signed [5:0] M_a_e,  M_a_e_next;
  reg signed [31:0] tmp;
  
  always @(posedge clk or negedge rst_n) begin
    if(rst_n == 0) begin
      data_act <= 8'd0;
      b_f <= 16'd0;
      M_w_m <= 16'd0;
      M_w_e <= 16'd0;
      M_a_m <= 16'd0;
      M_a_e <= 16'd0;
    end
    else begin
      data_act <= data_act_next;
      b_f <= b_f_next;
      M_w_m <= M_w_m_next;
      M_w_e <= M_w_e_next;
      M_a_m <= M_a_m_next;
      M_a_e <= M_a_e_next;
    end
      //$display("en_store=%b in=%d weight=%d psum_in=%d psum_out=%d", en_store, $signed(in), $signed(weight), $signed(psum_in), $signed(psum_out));
  end
     
  always @* begin
    data_act_next = data_act;
    b_f_next = b_f;
    M_w_m_next = M_w_m;
    M_w_e_next = M_w_e;
    M_a_m_next = M_a_m;
    M_a_e_next = M_a_e;
    tmp = 32'd0;
    case(op_type)
      2'd0: begin
        tmp = data_in * M_w_m;
        if(M_w_e < 0)
          tmp = tmp >> -M_w_e;
        else
          tmp = tmp << M_w_e;
        tmp = tmp + b_f;
        if(tmp < 0)
          tmp = 0;
        tmp = tmp * M_a_m;
        if(M_w_e < 0)
          tmp = tmp >> -M_a_e;
        else
          tmp = tmp << M_a_e;
        data_act_next = tmp[`p4_act-1:0];
      end
      2'd1: begin
        b_f_next = data_in[15:0];
      end
      2'd2: begin
        M_w_m_next = data_in[9:0];
        M_w_e_next = data_in[21:16];
      end
      2'd3: begin
        M_a_m_next = data_in[9:0];
        M_a_e_next = data_in[21:16];
      end
    endcase
  end

  
endmodule
