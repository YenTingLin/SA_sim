#include "skip.h"

Skip::Skip(const sc_module_name &mn, int c, int n_layer_skip)
  : sc_module(mn), C(c),
  i_fmap("i_fmap", c), o_fmap("o_Wsum", c){
  L = (CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE) * n_layer_skip;
  fifo = new sc_fifo< sc_int<IN_WIDTH> > *[C];
  for(int i_c = 0; i_c < C; i_c++)
    fifo[i_c] = new sc_fifo< sc_int<IN_WIDTH> >(L);
  SC_THREAD(data_transfer);
}

Skip::~Skip(){
  for(int i_c = 0; i_c < C; i_c++)
    delete fifo[i_c];
  delete fifo;
}

void Skip::fifo_reset(){
  //cout << "debug: Skip/fifo_reset: pre" << endl;
  for(int i_c = 0; i_c < C; i_c++){
    fifo[i_c]->write(0);
    //cout << "fifo[" << i_c << "].write(0);" << endl;
  }
  //wait(CLK_CYCLE, SC_NS);
  //cout << "debug: Skip/fifo_reset: pos" << endl;
}

void Skip::data_transfer(){
  fifo_reset();
  while(true){
    //cout << "debug: Skip/data_transfer_in: pre" << endl;
    for(int i_c = 0; i_c < C; i_c++){
      sc_int<IN_WIDTH> data_tmp;
      data_tmp = fifo[i_c]->read();
      o_fmap[i_c]->write(data_tmp);
      data_tmp = i_fmap[i_c]->read();
      fifo[i_c]->write(data_tmp);
    }
    wait(0, SC_NS);
    //cout << "debug: Skip/data_transfer_in: pos" << endl;
  }
}
