#ifndef _LAYER_H_
#define _LAYER_H_

#include "padding_in.h"
#include "synapse.h"
#include "padding_out.h"
#include "act.h"

SC_MODULE(Layer) {
public:
  Padding_in *padding_in;
  Synapse *synapse;
  Padding_out *padding_out;
  Act *act;
  
  sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > >  i_fmap;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<IN_WIDTH> > > > o_Wsum;
  //sc_port< sc_fifo_in_if< bool > > i_rst;
  
  string name;// layer name
  int C;      // input channel
  int K;      // output channel
  int X0;     // ifmap height
  int Y0;     // ifmap width
  int X;      // ofmap height
  int Y;      // ofmap width
  int C_last; // input channel from last layer
  int C_skip; // input channel for skip path
  int K_skip; // output channel for skip path
  int bit_mrg; // psum bit margin
  char Type;  // operation type {'C': conv, 'M': maxpooling, 'A': avgpooling} exit(1);
  long long int P_total, num_RRAM; // PEs, RRAM arrays count
  long long int cycle_cal_v, cycle_cal_inv; // cycle count
  Parallel_type ptype;
  Skip_type stype;
  int mul_C;  // parallel of input channel
  Array skip_cfg;
  int skip_L;
  
  SC_HAS_PROCESS(Layer);
  Layer(const sc_module_name &mn, int c, int k, int i, int j, int x0, int y0,
        bool en_zero_padding_in, int stride, bool en_DW, bool en_relu, int bit_mrg_,
        Parallel_type = Parallel_type::none, Array = a_null, Skip_type = Skip_type::st_none);
  Layer(const sc_module_name &mn, int c, int x0, int y0, int bit_mrg_, char pool_type);
  ~Layer();
  
  void map_filters(int ****Weights, int *bias, int *scale);
  void fetch_cycle_count();
  
private:
  sc_vector< sc_fifo< sc_int<IN_WIDTH> > > fifo_in;
  sc_vector< sc_fifo< sc_int<IN_WIDTH> > > fifo_in_2_syn;
  sc_vector< sc_fifo< sc_int<OUT_WIDTH> > > fifo_syn_2_out;
  sc_vector< sc_fifo< sc_int<OUT_WIDTH> > > fifo_out_2_act;
  sc_fifo< sc_int<IN_WIDTH> > **fifo_skips;
  static Array a_null;
  
  void fifo_reset();
  void data_transfer_in();
  void data_transfer_out();
  void maxpooling();
  void avgpooling();
};

#endif
