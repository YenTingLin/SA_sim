`timescale 1ns / 100ps

module stimulus;
  parameter cyc = 10;
  parameter delay = 1;

  reg clk, rst_n;
  reg [1:0] op_type;
  reg [31:0] data_in;
  wire [7:0] data_act;

// [HW] complete the port connections
  
  ACT act(clk, rst_n, op_type, data_in, data_act);
  
  always #(cyc/2) clk = ~clk;

  initial begin
    `ifdef SYNTHESIS
      $sdf_annotate("p4_act_syn.sdf", act);
      $fsdbDumpfile("p4_act_syn.fsdb");
    `else
      $fsdbDumpfile("p4_act.fsdb");
    `endif
    $fsdbDumpvars;
    
    $monitor($time, " clk=%b rst_n=%b op_type=%d data_in=%d | data_act=%d ",
      clk, rst_n, op_type, $signed(data_in), $signed(data_act));
  end

  initial begin
    clk = 1;
    rst_n = 1;
    op_type = 0;
    data_in = 0;
    
    #(cyc);
    #(delay) rst_n = 0;
    #(cyc*4) rst_n = 1;
    #(cyc*2);

    #(cyc) nop;
    #(cyc) load_b_f(50);
    #(cyc) load_M_w(1, -3);
    #(cyc) load_M_a(4, -1);
    #(cyc) active(64);
    #(cyc) load_b_f(487);
    #(cyc) load_M_w(32, -5);
    #(cyc) load_M_a(512, -15);
    #(cyc) active(513);
    #(cyc) nop;
    #(cyc) active(-294);
    #(cyc) nop;
    #(cyc) load_b_f(-7849);
    #(cyc) active(-2151);
    #(cyc) active(9999);
    #(cyc) nop;
    #(cyc) active(1183);
    #(cyc) load_M_w(128, -7);
    #(cyc) load_M_a(16, -10);
    #(cyc) active(-2151);
    #(cyc) active(9999);
    #(cyc) nop;
    #(cyc) active(1183);
    #(cyc) nop;
    
// [HW] apply more patterns to cover
// different conditions
    
    #(cyc*3);
    $finish;
  end

  // take a careful look at 
  // the usage of task here
  task nop;
    begin
      op_type = 0;
      data_in = 0;
    end
  endtask
  task active;
    input [31:0] data_i;
    begin
      op_type = 0;
      data_in = data_i;
    end
  endtask
  task load_b_f;
    input [15:0] data_b_f;
    begin
      op_type = 1;
      data_in = data_b_f;
    end
  endtask
  task load_M_w;
    input [15:0] data_m;
    input [15:0] data_e;
    begin
      op_type = 2;
      data_in = {data_e, data_m};
    end
  endtask
  task load_M_a;
    input [15:0] data_m;
    input [15:0] data_e;
    begin
      op_type = 3;
      data_in = {data_e, data_m};
    end
  endtask
endmodule
