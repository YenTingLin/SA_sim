#ifndef _P_OUT_H_
#define _P_OUT_H_

#include "config.h"

SC_MODULE(Padding_out){
public:
  sc_vector< sc_port< sc_fifo_in_if< sc_int<OUT_WIDTH> > > >  i_data;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<OUT_WIDTH> > > > o_data;

  int K;      // output channel
  int I;      // kernel height
  int J;      // kernel width
  int X0;     // ifmap height
  int Y0;     // ifmap width
  int St;     // stride
  long long int cycle_cal_v, cycle_cal_inv; // cycle count

  SC_HAS_PROCESS(Padding_out);
  Padding_out(const sc_module_name &mn, int k, int i, int j, int x0, int y0, int stride, int bit_mrg_, Parallel_type = Parallel_type::none);
  ~Padding_out(){}

  void map_scale(int *scale);
  
private:
  sc_int<SCALE_WIDTH> M_m_mul;
  sc_int<SCALE_WIDTH> M_m_sft;
  sc_int<64> data_tmp;

  const long long int num_max, num_min; // boundary of scale
  
  void data_transfer();
  void data_transfer_D();
  void data_transfer_2YS();
  void data_transfer_2XYS();
  void data_transfer_2Y();
  void data_transfer_2XY();
};

#endif
