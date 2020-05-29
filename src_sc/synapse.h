/************************
 * TODO:
 * *********************/
#ifndef _SYNAPSE_H_
#define _SYNAPSE_H_

#include "PE_array_CK.h"

SC_MODULE(Synapse){
public:
  sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > >   i_fmap;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<OUT_WIDTH> > > > o_Wsum;
  //sc_port< sc_fifo_in_if< bool > > i_rst;

  int C;      // input channel 
  int K;      // output channel
  int I;      // kernel height
  int J;      // kernel width
  int Y0;     // ifmap width
  bool en_DW; // depth wise
  long long int P_total, num_RRAM; // PEs, RRAM arrays count

  SC_HAS_PROCESS(Synapse);
  Synapse(const sc_module_name &mn, int c, int k, int i, int j, int y0, bool en_DW, int bit_mrg_, Parallel_type = Parallel_type::none);
  ~Synapse();

  void map_filters(int ****Filter);
  
private:
  sc_fifo< sc_int<IN_WIDTH> > ***fifo_is;
  sc_fifo< sc_int<IN_WIDTH> > ****fifo_js;
  PE_array_CK ***pe_array_CKs;
  sc_vector< sc_fifo< sc_int<IN_WIDTH> > > **vector_pe_ins;
  sc_vector< sc_fifo< sc_int<PSUM_WIDTH> > > **vector_pe_outs;
  static sc_vector< sc_fifo< sc_int<IN_WIDTH> > > v_null;
  
  struct Num{
    int iC, oK;
    int fiI, fiC, fiL;
    int fjI, fjJ, fjC;
    int pI, pJ, pC, pK;
  };

  Num num;

  const long long int max_psum; // 2^31 - 1
  const long long int min_psum; // -2^31
  const long long int max_out; // 2^7 - 1
  const long long int min_out; // -2^7
  const int bit_mrg;
  
  void build();
  void data_transfer();
  void data_transfer_D();
  void partial_sum();
  void data_transfer_2YS();
  void data_transfer_2XYS();
  void data_transfer_2Y();
  void data_transfer_2XY();
  void partial_sum_2Y();
  void partial_sum_2XY();
  //void debug(); //debug
  void fifo_reset();
  
  // f -> v...
  void data_transfer_C(sc_fifo< sc_int<IN_WIDTH> > **fifo_r, sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                             sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null);
  // f -> f, v...
  void data_transfer_C(sc_fifo< sc_int<IN_WIDTH> > **fifo_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w,
                                                             sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                             sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null);
  // f -> f, f, v...
  void data_transfer_C(sc_fifo< sc_int<IN_WIDTH> > **fifo_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w1,
                                                             sc_fifo< sc_int<IN_WIDTH> > **fifo_w2,
                                                             sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                             sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                             sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                             sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null);
  // p -> v...
  void data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null);
  // p -> f, v...
  void data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w1,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null);
  // p -> f, f, v...
  void data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w1,
                                                                                                       sc_fifo< sc_int<IN_WIDTH> > **fifo_w2,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null);
  // p -> f, f, f, v...
  void data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w1,
                                                                                                       sc_fifo< sc_int<IN_WIDTH> > **fifo_w2,
                                                                                                       sc_fifo< sc_int<IN_WIDTH> > **fifo_w3,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null,
                                                                                                       sc_vector< sc_fifo< sc_int<IN_WIDTH> > > & =v_null);
};

#endif
