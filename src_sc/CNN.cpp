#include "CNN.h"

CNN::CNN(const sc_module_name &mn, int mul_C, int c, int k, int x0, int y0)
  : sc_module(mn), C(c), K(k), X0(x0), Y0(y0), num_layers(0), mul_C(mul_C),
  i_fmap("i_fmap", mul_C*c), o_Wsum("o_Wsum", k){
  cout << "debug: CNN/CNN: pre" << endl;
  layers = new Layer *[NUM_LAYERS];
  fifo_layer_2_layer = new sc_vector< sc_fifo< sc_int<IN_WIDTH> > > *[NUM_LAYERS-1];
  
  def_CNN();

  X = layers[num_layers-1]->X;
  Y = layers[num_layers-1]->Y;
  for(int i_k = 0; i_k < K; i_k++){
    layers[num_layers-1]->o_Wsum[i_k](o_Wsum[i_k]);
  }
  
  cout << "debug: CNN/CNN: pos" << endl;
}

CNN::~CNN(){
  for(int i_l = 0; i_l < num_layers; i_l++){
    delete layers[i_l];
    if(i_l < num_layers-1)
      delete fifo_layer_2_layer[i_l];
  }
  delete [] layers;
  delete [] fifo_layer_2_layer;
}

void CNN::map_filters(int *****Weights, int **bias, int **scale){
  for(int i_l = 0; i_l < num_layers; i_l++){
    //cout << "debug: CNN/map_filters: layer" << i_l << endl;
    layers[i_l]->map_filters(Weights[i_l], bias[i_l], scale[i_l]);
  }
}

Layer_property *CNN::fetch_cycle_count(){
  Layer_property *properties = new Layer_property[num_layers];
  for(int i = 0; i < num_layers; i++){
    properties[i].name = layers[i]->name;
    switch(layers[i]->Type){
      case 'C':
        if(layers[i]->synapse->en_DW)
          properties[i].type = "conv2d_dw";
        else
          properties[i].type = "conv2d";
        properties[i].I = layers[i]->synapse->I;
        properties[i].J = layers[i]->synapse->J;
        properties[i].St = layers[i]->padding_out->St;
      break;
      case 'M':
        properties[i].type = "maxpooling";
      case 'A':
        properties[i].type = "avgpooling";
        properties[i].I = layers[i]->X0;
        properties[i].J = layers[i]->Y0;
        properties[i].St = layers[i]->X0;
      break;
    }
    properties[i].C = layers[i]->C;
    properties[i].K = layers[i]->K;
    properties[i].X0 = layers[i]->X0;
    properties[i].Y0 = layers[i]->Y0;
    properties[i].X = layers[i]->X;
    properties[i].Y = layers[i]->Y;
    properties[i].ptype = layers[i]->ptype;
    properties[i].P_total = layers[i]->P_total;
    properties[i].num_RRAM = layers[i]->num_RRAM;
    properties[i].skip_L = layers[i]->skip_L;
    properties[i].skip_W = IN_WIDTH * layers[i]->mul_C * layers[i]->K_skip;
    layers[i]->fetch_cycle_count();
    properties[i].cycle_cal_v = layers[i]->cycle_cal_v;
    properties[i].cycle_cal_inv = layers[i]->cycle_cal_inv;
  }
  return properties;
}

int *CNN::get_parameters(){
  //cout << "debug: CNN/get_parameters: pre" << endl;
  if(num_layers == NUM_LAYERS){
    cout << "Error: CNN::add_conv_layer: num_layers(" << num_layers << ") out of range NUM_LAYERS(" << NUM_LAYERS << ")" << endl;
    exit(1);
  }
  int *out = new int[3];
  if(num_layers == 0){
    out[0] = C;
    out[1] = X0;
    out[2] = Y0;
  }
  else{
    out[0] = layers[num_layers-1]->K; // C
    out[1] = layers[num_layers-1]->X; // X0
    out[2] = layers[num_layers-1]->Y; // Y0
  }
  //cout << "debug: CNN/get_parameters: pos" << endl;
  return out;
}

Array CNN::get_skips(Skip_type stype){
  Skip_type stype_o = Skip_type::st_none;
  if(num_layers != 0)
    stype_o = layers[num_layers-1]->stype;
  if(stype_o == st_none || stype_o == st_deg){
    if(stype == st_ext){
      Array a_skip(1);
      if(num_layers == 0)
        a_skip.data[0] = C;
      else
        a_skip.data[0] = layers[num_layers-1]->K;
      cout << "a_skip = "; a_skip.print();
      return a_skip;
    }
    else{
      Array a_skip(0);
      cout << "a_skip = "; a_skip.print();
      return a_skip;
    }
  }
  else{
    if(stype == st_ext){
      Array a_skip_o = layers[num_layers-1]->skip_cfg;
      Array a_skip(a_skip_o.len + 1);
      a_skip.fill(a_skip_o);
      a_skip.data[a_skip_o.len] = layers[num_layers-1]->K;
      cout << "a_skip = "; a_skip.print();
      return a_skip;
    }
    else{
      Array a_skip = layers[num_layers-1]->skip_cfg;
      cout << "a_skip = "; a_skip.print();
      return a_skip;
    }
  }
}

void CNN::port_connection(){
  cout << "debug: CNN/port_connection: pre" << endl;
  if(num_layers == 0){
    for(int i_c = 0; i_c < mul_C*C; i_c++){
      layers[num_layers]->i_fmap[i_c](i_fmap[i_c]);
    }
  }
  else{
    char str_[20];
    sprintf(str_, "fifo_%d,%d", num_layers-1, num_layers);
    int C_ = layers[num_layers]->C_last + layers[num_layers]->C_skip;
    //cout << "layers[" << num_layers << "]->C_last = " << layers[num_layers]->C_last << "\nlayers[" << num_layers << "]->C_skip = " << layers[num_layers]->C_skip << endl; //debug
    int mul_C_ = layers[num_layers]->mul_C;
    fifo_layer_2_layer[num_layers-1] = new sc_vector< sc_fifo< sc_int<IN_WIDTH> > >(str_, mul_C_*C_);
    //cout << "mul_C_*C_ = " << mul_C_*C_ << endl; //debug
    for(int i_c = 0; i_c < mul_C_*C_; i_c++){
      layers[num_layers-1]->o_Wsum[i_c]((*fifo_layer_2_layer[num_layers-1])[i_c]);
      layers[num_layers]->i_fmap[i_c]((*fifo_layer_2_layer[num_layers-1])[i_c]);
      //cout << "layers[" << num_layers-1 << "]->o_Wsum[" << i_c << "]" << endl;
    }
  }
  cout << "debug: CNN/port_connection: pos" << endl;
  //layers[num_layers]->i_rst(layer_rsts[num_layers]);
}

void CNN::add_conv_layer(int k, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, int bit_mrg, string name, Parallel_type ptype){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  layers[num_layers] = new Layer(str_, out[0], k, i, j, out[1], out[2], en_zero_padding_in, stride, false, en_relu, bit_mrg, ptype);
  port_connection();
  delete [] out;
  num_layers++;
}

void CNN::add_conv_layer_skip(int k, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, Skip_type stype, int bit_mrg, string name, Parallel_type ptype){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  Array a_skip = get_skips(stype);
  layers[num_layers] = new Layer(str_, out[0], k, i, j, out[1], out[2], en_zero_padding_in, stride, false, en_relu, bit_mrg, ptype, a_skip, stype);
  port_connection();
  delete [] out;
  num_layers++;
}

void CNN::add_conv_layer_mul(float mul, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, int bit_mrg, string name, Parallel_type ptype){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  layers[num_layers] = new Layer(str_, out[0], mul * out[0], i, j, out[1], out[2], en_zero_padding_in, stride, false, en_relu, bit_mrg, ptype);
  port_connection();
  delete [] out;
  num_layers++;
}

void CNN::add_conv_layer_mul_skip(float mul, int i, int j, bool en_zero_padding_in, int stride, bool en_relu, Skip_type stype, int bit_mrg, string name, Parallel_type ptype){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  Array a_skip = get_skips(stype);
  int K_;
  switch(stype){
    case st_ext:
      K_ = mul * (out[0] + a_skip.sum_1());
      break;
    case st_par:
      K_ = mul * out[0];
      break;
    case st_deg:
      K_ = mul * (out[0] + a_skip.sum());
      break;
    default:
      K_ = mul * out[0];
  }
  layers[num_layers] = new Layer(str_, out[0], K_, i, j, out[1], out[2], en_zero_padding_in, stride, false, en_relu, bit_mrg, ptype, a_skip, stype);
  port_connection();
  delete [] out;
  num_layers++;
}

void CNN::add_DWconv_layer(int i, int j, bool en_zero_padding_in, int stride, bool en_relu, int bit_mrg, string name, Parallel_type ptype){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  layers[num_layers] = new Layer(str_, out[0], out[0], i, j, out[1], out[2], en_zero_padding_in, stride, true, en_relu, bit_mrg, ptype);
  port_connection();
  delete [] out;
  num_layers++;
}

void CNN::add_FC_layer(int k, bool en_relu, int bit_mrg, string name, Parallel_type ptype){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  if(out[1] != 1 || out[2] != 1){
    cout << "Error: CNN::add_FC_layer: the size of ifmap must be 1x1" << endl;
    exit(9487);
  }
  layers[num_layers] = new Layer(str_, out[0], k, 1, 1, 1, 1, false, 1, false, en_relu, bit_mrg, ptype);
  port_connection();
  delete [] out;
  num_layers++;
}

void CNN::add_maxpooling_layer(int bit_mrg, string name){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  Array a_skip = get_skips(Skip_type::st_deg);
  layers[num_layers] = new Layer(str_, out[0] + a_skip.sum(), out[1], out[2], bit_mrg, 'M');
  port_connection();
  delete [] out;
  num_layers++;
}

void CNN::add_avgpooling_layer(int bit_mrg, string name){
  const char *str_;
  str_ = ("layer_" + name).c_str();
  int *out = get_parameters();
  Array a_skip = get_skips(Skip_type::st_deg);
  layers[num_layers] = new Layer(str_, out[0] + a_skip.sum(), out[1], out[2], bit_mrg, 'A');
  port_connection();
  delete [] out;
  num_layers++;
}
