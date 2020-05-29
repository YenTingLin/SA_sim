`timescale 1ns / 100ps

module stimulus;
  parameter cyc = 10;
  parameter delay = 1;

  reg clk, rst_n, en_store;
  reg [7:0] in;
  reg [31:0] psum_in;
  wire [31:0] psum_out;

// [HW] complete the port connections
  
  PE pe(clk, rst_n, en_store, in, psum_in, psum_out);
  
  always #(cyc/2) clk = ~clk;

  initial begin
    `ifdef SYNTHESIS
      $sdf_annotate("p1_PE_syn.sdf", pe);
      $fsdbDumpfile("p1_PE_syn.fsdb");
    `else
      $fsdbDumpfile("p1_PE.fsdb");
    `endif
    $fsdbDumpvars;
    
    $monitor($time, " clk=%b rst_n=%b en_store=%b in=%d psum_in=%d | psum_out=%d ",
      clk, rst_n, en_store, $signed(in), $signed(psum_in), $signed(psum_out));
  end

  initial begin
    clk = 1;
    rst_n = 1;
    en_store = 0;
    in = 0;
    psum_in = 0;
    
    #(cyc);
    #(delay) rst_n = 0;
    #(cyc*4) rst_n = 1;
    #(cyc*2);

    #(cyc) nop;
    #(cyc) load(8'd94);
    #(cyc) data_in(8'd87, 32'd4265);
    #(cyc) data_in(-8'd78, 32'd4265);
    #(cyc) nop;
    #(cyc) data_in(8'd78, -32'd38426);
    #(cyc) load(-8'd49);
    #(cyc) nop;
    #(cyc) data_in(-8'd32, 32'd4265);
    #(cyc) data_in(-8'd127, -32'd453276);
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
      en_store = 0;
      in = 0;
      psum_in = 0;
    end
  endtask
  task load;
    input [7:0] data1;
    begin
      en_store = 1;
      in = data1;
      psum_in = 0;
    end
  endtask
  task data_in;
    input [7:0] data1;
    input [31:0] data2;
    begin
      en_store = 0;
      in = data1;
      psum_in = data2;
    end
  endtask
endmodule
