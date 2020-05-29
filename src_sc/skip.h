#ifndef _SKIP_H_
#define _SKIP_H_

#include "config.h"

SC_MODULE(Skip) {
public:
  sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > >  i_fmap;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<IN_WIDTH> > > > o_fmap;

  int C;      // input channel
  int L;      // fifo depth
  
  SC_HAS_PROCESS(Skip);
  Skip(const sc_module_name &mn, int c, int n_layer_skip=1);
  ~Skip();
  
private:
  sc_fifo< sc_int<IN_WIDTH> > **fifo;
  
  void fifo_reset();
  void data_transfer();
  //void data_transfer_in();
  //void data_transfer_out();
};

#endif
