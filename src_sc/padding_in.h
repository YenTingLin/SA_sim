#ifndef _P_IN_H_
#define _P_IN_H_

#include "config.h"

SC_MODULE(Padding_in){
public:
  sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > >  i_data;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<IN_WIDTH> > > > o_data;

  int C;      // input channel 
  int I;      // kernel height
  int J;      // kernel width
  int X0;     // ifmap height
  int Y0;     // ifmap width
  bool en_zero_padding;

  SC_HAS_PROCESS(Padding_in);
  Padding_in(const sc_module_name &mn, int c, int i, int j, int x0, int y0, bool en_zero_padding_in, Parallel_type = Parallel_type::none);
  ~Padding_in(){}
  
private:
  sc_int<IN_WIDTH> tmp_data;
  
  void data_transfer();
  void data_transfer_D();
  void data_transfer_2YS();
  void data_transfer_2XYS();
  void data_transfer_2Y();
  void data_transfer_2XY();
};

#endif
