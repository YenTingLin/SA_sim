`timescale 1ns / 100ps

module stimulus;
  parameter cyc = 10;
  parameter delay = 1;

  reg clk, rst_n, active;
  reg [7:0] data_in;
  wire done;
  wire [7:0] data_out;
  reg [7:0] a_data_in [0:`p3_cycle+5];
  integer i;

// [HW] complete the port connections
  
  AVG avg(clk, rst_n, active, data_in, done, data_out);
  
  always #(cyc/2) clk = ~clk;

  initial begin
    `ifdef SYNTHESIS
      $sdf_annotate("p3_avg_syn.sdf", avg);
      $fsdbDumpfile("p3_avg_syn.fsdb");
    `else
      $fsdbDumpfile("p3_avg.fsdb");
    `endif
    $fsdbDumpvars;
    
    $monitor($time, " clk=%b rst_n=%b active=%b data_in=%d done=%b | data_out=%d ",
      clk, rst_n, active, $signed(data_in), done, $signed(data_out));
  end

  initial begin
    clk = 1;
    rst_n = 1;
    active = 0;
    data_in = 0;
    $readmemh("data_in_h.txt", a_data_in);
    
    #(cyc);
    #(delay) rst_n = 0;
    #(cyc*4) rst_n = 1;
    #(cyc*2);

    #(cyc) nop;
    for(i = 0; i < `p3_cycle / 2; i=i+1) begin
      #(cyc) load(a_data_in[i]);
    end
    #(cyc) nop;
    #(cyc) nop;
    for(i = `p3_cycle / 2; i < `p3_cycle; i=i+1) begin
      #(cyc) load(a_data_in[i]);
    end
    #(cyc) nop;
    #(cyc) load(a_data_in[`p3_cycle]);
    #(cyc) load(a_data_in[`p3_cycle+1]);
    #(cyc) load(a_data_in[`p3_cycle+2]);
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
      active = 0;
      data_in = 0;
    end
  endtask
  task load;
    input [7:0] data_i;
    begin
      active = 1;
      data_in = data_i;
    end
  endtask
endmodule
