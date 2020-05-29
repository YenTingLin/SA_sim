#include "PE_array_CK.h"

PE_array_CK::PE_array_CK(const sc_module_name &mn, int c, int k, bool en_DW_in, int bit_mrg_)
  : sc_module(mn), C(c), K(k), en_DW(en_DW_in),
  max_psum((1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1)) - 1),
  min_psum(-(1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1))),
  i_fmap("i_fmap", c), o_Wsum("o_Wsum",k){
  if(en_DW){
    weights = new sc_int<WEIGHT_WIDTH> *[1];
    weights[0] = new sc_int<WEIGHT_WIDTH> [C];
    P_total = C;

    SC_THREAD(vector_product_DW);
  }
  else{
    reg_i_fmap = new sc_int<IN_WIDTH>[C];
    weights = new sc_int<WEIGHT_WIDTH> *[C];
    for(int i_c = 0; i_c < C; i_c++)
      weights[i_c] = new sc_int<WEIGHT_WIDTH> [K];
    P_total = C * K;

    SC_THREAD(vector_product);
  }
}

PE_array_CK::~PE_array_CK(){
  if(en_DW)
    delete [] weights[0];
  else{
    for(int i_c = 0; i_c < C; i_c++)
      delete [] weights[i_c];
    delete [] reg_i_fmap;
  }
  delete [] weights;
}

void PE_array_CK::map_filters(int** Filter){
  for(int i_c = 0; i_c < C; i_c++){
    if(en_DW)
      weights[0][i_c] = Filter[i_c][0];
    else
      for(int i_k = 0; i_k < K; i_k++)
        weights[i_c][i_k] = Filter[i_c][i_k];
  }
}
/**/
void PE_array_CK::vector_product(){
  while(true){
    //cout << "debug: PE_array_CK/vector_product: pre" << endl;
    for(int i_c = 0; i_c < C; i_c++)
      reg_i_fmap[i_c] = i_fmap[i_c]->read();
    for(int i_k = 0; i_k < K; i_k++){
      sc_int<64> psum = 0;
      for(int i_c = 0; i_c < C; i_c++){
        psum +=  reg_i_fmap[i_c] * weights[i_c][i_k];
        if(psum > max_psum)
          psum = max_psum;
        else if(psum < min_psum)
          psum = min_psum;
      }
      o_Wsum[i_k]->write(psum);
    }
    wait(CLK_CYCLE, SC_NS);
    //cout << "debug: PE_array_CK/vector_product: pos" << endl;
  }
}

void PE_array_CK::vector_product_DW(){
  while(true){
    //cout << "debug: PE_array_CK/vector_product: pre" << endl;
    for(int i_c = 0; i_c < C; i_c++){
      o_Wsum[i_c]->write(i_fmap[i_c]->read() * weights[0][i_c]);
    }
    wait(CLK_CYCLE, SC_NS);
    //cout << "debug: PE_array_CK/vector_product: pos" << endl;
  }
}
