#ifndef _TESTBENCH_H_
#define _TESTBENCH_H_

#include "config.h"

SC_MODULE(Testbench){
public:
  sc_vector< sc_port< sc_fifo_out_if< sc_int<IN_WIDTH> > > > o_fmap;
  sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > i_Wsum;
  //sc_port< sc_fifo_out_if< bool > > o_rst;
  
  int L;      // number of layers
  int C;      // input channel 
  int K;      // output channel
  //int X0;     // ifmap height
  //int Y0;     // ifmap width
  int B; // batch size
  int mul_C;  // parallel of input channel
  
  int *****weights;
  int **shape_weights;
  int **bias;
  int *shape_bias;
  int **scale;
  int L_real;
  long long int cycle_total; // cycle count

  SC_HAS_PROCESS(Testbench);
  Testbench(const sc_module_name &mn, int mul_C, int num_layers, int c, int k);
  ~Testbench();

  void read_filter();
  void dump_cycle_count(Layer_property *properties, int num_layers);
  
private:
  sc_time t_begin;
  
  void read_ifmap();
  void read_ifmap_2();
  void read_ifmap_4();
  void dump_ofmap();
};

ostream &operator<<(ostream &sout, Parallel_type ptype);

#endif