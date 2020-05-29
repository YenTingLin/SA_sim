#include "CNN.h"

void CNN::def_TheAllConvNet(){
  bool padding = true; //true
  int K1 = 96;//8//96
  int K2 = 192;//16//192

  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[0], "c0"); // layer #0
  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[1], "c1"); // layer #1
  add_conv_layer(K1, 3, 3, padding, 2, true, bit_mrg[2], "c2"); // layer #2
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[3], "c3"); // layer #3
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[4], "c4"); // layer #4
  add_conv_layer(K2, 3, 3, padding, 2, true, bit_mrg[5], "c5"); // layer #5
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[6], "c6"); // layer #6
  add_conv_layer(K2, 1, 1, padding, 1, true, bit_mrg[7], "c7"); // layer #7
  add_conv_layer(K, 1, 1, padding, 1, true, bit_mrg[8], "c8"); // layer #8
  add_avgpooling_layer(bit_mrg[9], "ap9"); // layer #9
}

void CNN::def_TheAllConvNet_e1(){
  bool padding = true; //true
  int K1 = 96;//8//96
  int K2 = 192;//16//192

  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[0], "c0", Parallel_type::none); // layer #0
  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[1], "c1", Parallel_type::none); // layer #1
  add_conv_layer(K1, 3, 3, padding, 2, true, bit_mrg[2], "c2", Parallel_type::pt_2YS); // layer #2
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[3], "c3", Parallel_type::none); // layer #3
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[4], "c4", Parallel_type::none); // layer #4
  add_conv_layer(K2, 3, 3, padding, 2, true, bit_mrg[5], "c5", Parallel_type::pt_2YS); // layer #5
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[6], "c6", Parallel_type::none); // layer #6
  add_conv_layer(K2, 1, 1, padding, 1, true, bit_mrg[7], "c7", Parallel_type::none); // layer #7
  add_conv_layer(K, 1, 1, padding, 1, true, bit_mrg[8], "c8", Parallel_type::none); // layer #8
  add_avgpooling_layer(bit_mrg[9], "ap9"); // layer #9
}

void CNN::def_TheAllConvNet_e2(){
  bool padding = true; //true
  int K1 = 96;//8//96
  int K2 = 192;//16//192

  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[0], "c0", Parallel_type::pt_2Y); // layer #0
  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[1], "c1", Parallel_type::pt_2Y); // layer #1
  add_conv_layer(K1, 3, 3, padding, 2, true, bit_mrg[2], "c2", Parallel_type::pt_2Ym); // layer #2
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[3], "c3", Parallel_type::none); // layer #3
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[4], "c4", Parallel_type::none); // layer #4
  add_conv_layer(K2, 3, 3, padding, 2, true, bit_mrg[5], "c5", Parallel_type::pt_2YS); // layer #5
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[6], "c6", Parallel_type::none); // layer #6
  add_conv_layer(K2, 1, 1, padding, 1, true, bit_mrg[7], "c7", Parallel_type::none); // layer #7
  add_conv_layer(K, 1, 1, padding, 1, true, bit_mrg[8], "c8", Parallel_type::none); // layer #8
  add_avgpooling_layer(bit_mrg[9], "ap9"); // layer #9
}

void CNN::def_TheAllConvNet_e4(){
  bool padding = true; //true
  int K1 = 96;//8//96
  int K2 = 192;//16//192

  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[0], "c0", Parallel_type::none); // layer #0
  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[1], "c1", Parallel_type::none); // layer #1
  add_conv_layer(K1, 3, 3, padding, 2, true, bit_mrg[2], "c2", Parallel_type::pt_2XYS); // layer #2
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[3], "c3", Parallel_type::none); // layer #3
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[4], "c4", Parallel_type::none); // layer #4
  add_conv_layer(K2, 3, 3, padding, 2, true, bit_mrg[5], "c5", Parallel_type::pt_2XYS); // layer #5
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[6], "c6", Parallel_type::none); // layer #6
  add_conv_layer(K2, 1, 1, padding, 1, true, bit_mrg[7], "c7", Parallel_type::none); // layer #7
  add_conv_layer(K, 1, 1, padding, 1, true, bit_mrg[8], "c8", Parallel_type::none); // layer #8
  add_avgpooling_layer(bit_mrg[9], "ap9"); // layer #9
}

void CNN::def_TheAllConvNet_e5(){
  bool padding = true; //true
  int K1 = 96;//8//96
  int K2 = 192;//16//192

  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[0], "c0", Parallel_type::pt_2XY); // layer #0
  add_conv_layer(K1, 3, 3, padding, 1, true, bit_mrg[1], "c1", Parallel_type::pt_2XY); // layer #1
  add_conv_layer(K1, 3, 3, padding, 2, true, bit_mrg[2], "c2", Parallel_type::pt_2XYm); // layer #2
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[3], "c3", Parallel_type::none); // layer #3
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[4], "c4", Parallel_type::none); // layer #4
  add_conv_layer(K2, 3, 3, padding, 2, true, bit_mrg[5], "c5", Parallel_type::pt_2XYS); // layer #5
  add_conv_layer(K2, 3, 3, padding, 1, true, bit_mrg[6], "c6", Parallel_type::none); // layer #6
  add_conv_layer(K2, 1, 1, padding, 1, true, bit_mrg[7], "c7", Parallel_type::none); // layer #7
  add_conv_layer(K, 1, 1, padding, 1, true, bit_mrg[8], "c8", Parallel_type::none); // layer #8
  add_avgpooling_layer(bit_mrg[9], "ap9"); // layer #9
}
