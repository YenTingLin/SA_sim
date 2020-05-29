#ifndef _CNN_H_
#define _CNN_H_

#include "layer.h"

#define NUM_LAYERS 200
//#define N_CLASS 10

SC_MODULE(CNN) {
public:
  Layer **layers;
  
  sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > >  i_fmap;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<IN_WIDTH> > > > o_Wsum;
  
  int C;      // input channel
  int K;      // output channel
  int X0;     // ifmap height
  int Y0;     // ifmap width
  int X;      // ofmap height
  int Y;      // ofmap width
  int num_layers;
  int mul_C;  // parallel of input channel
  
  SC_HAS_PROCESS(CNN);
  CNN(const sc_module_name &mn, int mul_C, int c, int k, int x0, int y0);
  ~CNN();
  
  void map_filters(int *****Weights, int **bias, int **scale);
  Layer_property *fetch_cycle_count();
  
private:
  sc_vector< sc_fifo< sc_int<IN_WIDTH> > > **fifo_layer_2_layer;
  
  int *get_parameters();
  Array get_skips(Skip_type stype);
  void port_connection();
  void add_conv_layer(int k, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, int bit_mrg, string name, Parallel_type = Parallel_type::none); // using
  void add_conv_layer_skip(int k, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, Skip_type stype, int bit_mrg, string name, Parallel_type = Parallel_type::none);
  void add_conv_layer_mul(float mul, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, int bit_mrg, string name, Parallel_type = Parallel_type::none);
  void add_conv_layer_mul_skip(float mul, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, Skip_type stype, int bit_mrg, string name, Parallel_type = Parallel_type::none);
  void add_DWconv_layer(int i, int j, bool en_zero_padding_in, int stride, bool en_relu, int bit_mrg, string name, Parallel_type = Parallel_type::none);
  void add_FC_layer(int k, bool en_relu, int bit_mrg, string name, Parallel_type = Parallel_type::none);
  void add_maxpooling_layer(int bit_mrg, string name);
  void add_avgpooling_layer(int bit_mrg, string name); // using

  // sub network
  void add_DWPWconv_block(int k, int ij, bool en_zero_padding_in, int stride, int duplication, bool *en_relus, string name); // (2*n)-layer sub network: {DWconv, PWconv} * n
  void add_MBconv_block(int k, int mul, int ij, bool en_zero_padding_in, int stride, int duplication, bool *en_relus, string name); // (3*n)-layer sub network: {PWconv, DWconv, PWconv} * n
  void add_131conv_block(int k1, int k2, int ij, bool en_zero_padding_in, int stride, int duplication, bool *en_relus, string name); // (3*n)-layer sub network: {conv_1x1, conv_3x3, conv_1x1} * n
  void add_Dense_block(int num_units, int k, bool en_bottleneck, string name, Parallel_type = Parallel_type::none); // (2*n)-layer sub network: {PWconv, conv} * n
  void add_Transition_block(float theta, string name, Parallel_type = Parallel_type::none); // 2-layer sub network: {PWconv, Avg}

  // DNN instance
  void def_CNN();
};

#endif
