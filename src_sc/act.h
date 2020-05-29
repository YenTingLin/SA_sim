#ifndef _ACT_H_
#define _ACT_H_

#include "config.h"

SC_MODULE(Act){
public:
  sc_vector< sc_port< sc_fifo_in_if< sc_int<OUT_WIDTH> > > > i_data;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<IN_WIDTH> > > > o_data;

  int K; // output channel
  bool en_relu;
  long long int num_max; // upper bound of relu
  
  SC_HAS_PROCESS(Act);
  Act(const sc_module_name &mn, int k, long long int num_max_in, Parallel_type = Parallel_type::none);
  Act(const sc_module_name &mn, int k, bool en_relu_in, Parallel_type = Parallel_type::none);
  ~Act();

  void map_bias(int *bias_in, int *scale);
  
private:
  sc_int<BIAS_WIDTH> *bias;
  sc_int<SCALE_WIDTH> M_b_mul;
  sc_int<SCALE_WIDTH> M_b_sft;
  sc_int<64> data_tmp;

  int mul_K;
  
  void relu();
};

#endif
