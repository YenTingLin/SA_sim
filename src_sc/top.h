#ifndef _TOP_H_
#define _TOP_H_

#include "CNN.h"
#include "testbench.h"

/********** The All Conv Net **********/
#define C  3      // input channel
#define K  10     // output channel
#define X0 32     // ifmap height
#define Y0 32     // ifmap width
/*
#if DNN_TYPE % 10 == 9
#define mul_C 1 // debug parallelism
#elif DNN_TYPE % 10 == 2
#define mul_C 2 // dual parallelism
#elif DNN_TYPE % 10 == 5
#define mul_C 4 // quad parallelism
#else
#define mul_C 1 // singal parallelism
#endif
*/
SC_MODULE(top) {

  CNN cnn;
  Testbench testbench;
  
  sc_vector< sc_fifo< sc_int<IN_WIDTH> > > fifo_fmap;
  sc_vector< sc_fifo< sc_int<IN_WIDTH> > > fifo_Wsum;
  //sc_fifo< bool > fifo_rst;
  
	SC_HAS_PROCESS(top);
  top(const sc_module_name &mn)
  : sc_module(mn), cnn( "cnn", mul_C_in, C, K, X0, Y0), testbench("test", mul_C_in, NUM_LAYERS, C, K),
  fifo_fmap("fifo_fmap", mul_C_in * C), fifo_Wsum("fifo_Wsum", K){
  	cout << "debug: top/top: pre" << endl;
    //cnn.i_rst(fifo_rst);
    //testbench.o_rst(fifo_rst);
    cout << "debug: top/top: 1" << endl;
    for(int i_c = 0; i_c < mul_C_in * C; i_c++){
      cnn.i_fmap[i_c](fifo_fmap[i_c]);
      testbench.o_fmap[i_c](fifo_fmap[i_c]);
    }
    cout << "debug: top/top: 2" << endl;
    for(int i_k = 0; i_k < K; i_k++){
      cnn.o_Wsum[i_k](fifo_Wsum[i_k]);
      testbench.i_Wsum[i_k](fifo_Wsum[i_k]);
    }
    cout << "debug: top/top: 3" << endl;
    cnn.map_filters(testbench.weights, testbench.bias, testbench.scale);
    cout << "debug: top/top: pos" << endl;
  }

  void dump_cycle_count(){
    testbench.dump_cycle_count(cnn.fetch_cycle_count(), cnn.num_layers);
  }
};

#endif
