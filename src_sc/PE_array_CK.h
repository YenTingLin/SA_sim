#ifndef _PE_ARRAY_CK_H_
#define _PE_ARRAY_CK_H_

#include "config.h"

SC_MODULE(PE_array_CK){
public:
  sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > >    i_fmap;
  sc_vector< sc_port< sc_fifo_out_if< sc_int<PSUM_WIDTH> > > > o_Wsum;

  long long int P_total; // PEs count

  SC_HAS_PROCESS(PE_array_CK);
  PE_array_CK(const sc_module_name &mn, int c, int k, bool en_DW_in, int bit_mrg_);
  ~PE_array_CK();

  void map_filters(int** Filter);
  
private:
  int C;      // input channel 
  int K;      // output channel
  bool en_DW; // depth wise
  
  sc_int<WEIGHT_WIDTH> **weights;
  sc_int<IN_WIDTH> *reg_i_fmap;
  
  void vector_product();
  void vector_product_DW();
  
  const long long int max_psum; // 2^31 - 1
  const long long int min_psum; // -2^31
};

#endif
