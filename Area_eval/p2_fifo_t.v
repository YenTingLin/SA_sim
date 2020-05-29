`timescale 1ns / 100ps

module stimulus;
  parameter cyc = 10;
  parameter delay = 1;

  reg clk;
  reg rst_n;
  reg en_read;
  reg en_write;
  reg [`p2_width-1:0] data_in;
  wire is_full;
  wire is_empty;
  wire [ `p2_width-1:0] data_out;

// [HW] complete the port connections
  
  FIFO fifo(clk, rst_n, en_read, en_write, data_in, is_full, is_empty, data_out);
  
  always #(cyc/2) clk = ~clk;

  initial begin
    `ifdef SYNTHESIS
      $sdf_annotate("p2_fifo_syn.sdf", fifo);
      $fsdbDumpfile("p2_fifo_syn.fsdb");
    `else
      $fsdbDumpfile("p2_fifo.fsdb");
    `endif
    $fsdbDumpvars;
    
    $monitor($time, " clk=%b rst_n=%b en_read=%b en_write=%b data_in=%d is_full=%b is_empty=%b | data_out=%d ",
      clk, rst_n, en_read, en_write, $signed(data_in), is_full, is_empty, $signed(data_out));
  end

  initial begin
    clk = 1;
    rst_n = 1;
    en_read = 0;
    en_write = 0;
    data_in = 0;
    
    #(cyc);
    #(delay) rst_n = 0;
    #(cyc*4) rst_n = 1;
    #(cyc*2);

    #(cyc) nop;
    #(cyc) push(8'd94);
    #(cyc) push(8'd87);
    #(cyc) push(-8'd49);
    #(cyc) pop;
    #(cyc) nop;
    #(cyc) pop_push(-8'd36);
    #(cyc) pop;
    #(cyc) push(-8'd49);
    #(cyc) nop;
    #(cyc) push(-8'd78);
    #(cyc) push(8'd21);
    #(cyc) pop_push(-8'd36);
    #(cyc) nop;
    #(cyc) pop_push(-8'd37);
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) push(8'd1);
    #(cyc) push(8'd2);
    #(cyc) push(8'd3);
    #(cyc) push(8'd4);
    #(cyc) push(8'd5);
    #(cyc) push(8'd6);
    #(cyc) push(8'd7);
    #(cyc) push(8'd8);
    #(cyc) pop_push(8'd9);
    #(cyc) pop_push(8'd10);
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
    #(cyc) pop;
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
      en_read = 0;
      en_write = 0;
      data_in = 0;
    end
  endtask
  task pop;
    begin
      en_read = 1;
      en_write = 0;
      data_in = 0;
    end
  endtask
  task push;
    input [`p2_width-1:0] data_i;
    begin
      en_read = 0;
      en_write = 1;
      data_in = data_i;
    end
  endtask
  task pop_push;
    input [`p2_width-1:0] data_i;
    begin
      en_read = 1;
      en_write = 1;
      data_in = data_i;
    end
  endtask
endmodule
