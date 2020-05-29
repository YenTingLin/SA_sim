#include "act.h"

#define DEF_1 \
  switch(ptype){ \
    case pt_2Y: \
      mul_K = 2; break; \
    case pt_2XY: \
      mul_K = 4; break; \
    default: \
      mul_K = 1; \
  } \
  i_data.init(mul_K * K); \
  o_data.init(mul_K * K); \
  bias = new sc_int<BIAS_WIDTH> [mul_K * K]

Act::Act(const sc_module_name &mn, int k, long long int num_max_in, Parallel_type ptype)
  : sc_module(mn), K(k), en_relu(true), num_max(num_max_in),
  i_data("i_data"), o_data("o_data"){
  //cout << "debug: Act/Act: pre" << endl;
  DEF_1;
  SC_THREAD(relu);
  //cout << "debug: Act/Act: pos" << endl;
}

Act::Act(const sc_module_name &mn, int k, bool en_relu_in, Parallel_type ptype)
  : sc_module(mn), K(k), en_relu(en_relu_in),
  i_data("i_data"), o_data("o_data"){
  //cout << "debug: Act/Act: pre" << endl;
  DEF_1;
  num_max = (1LL << (IN_WIDTH - 1)) - 1; // 2^7 - 1
  SC_THREAD(relu);
  //cout << "debug: Act/Act: pos" << endl;
}

Act::~Act(){
  delete [] bias;
}

void Act::map_bias(int *bias_in, int *scale){
  for(int i_k = 0; i_k < mul_K * K; i_k++){
    //cout << "debug: Act/map_bias: " << i_k << endl;
    //cout << "debug: bias_in["<< i_k <<"]=" << bias_in[i_k] << endl;
    bias[i_k] = bias_in[i_k % K];
  }
  M_b_mul = scale[0];
  M_b_sft = scale[1];
}

void Act::relu(){
  while(true){
    //cout << "debug: Act/relu: pre" << endl;
    for(int i_k = 0; i_k < mul_K * K; i_k++){
      //cout << "debug: Act/relu: 1" << endl;
      data_tmp = i_data[i_k]->read() + bias[i_k];
      //cout << "debug: Act/relu: 2" << endl;
      if(en_relu && data_tmp < 0)
        data_tmp = 0;
      /**/
      data_tmp *= M_b_mul;
      if(M_b_sft > 0)
        data_tmp = data_tmp << M_b_sft;
      else if(M_b_sft < 0)
        data_tmp = data_tmp >> (-M_b_sft);
      /**/
      if(data_tmp > num_max)
        data_tmp = num_max;

      o_data[i_k]->write(data_tmp);
      //cout << "debug: Act/relu: 3" << endl;
    }
    wait(ACT_CYCLE, SC_NS);
    //cout << "debug: Act/relu: pos" << endl;
  }
}
