#include "layer.h"

Array Layer::a_null(0);

Layer::Layer(const sc_module_name &mn, int c, int k, int i, int j, int x0, int y0, bool en_zero_padding_in, int stride, bool en_DW, bool en_relu, int bit_mrg_, Parallel_type ptype, Array a_skip, Skip_type stype)
  : sc_module(mn), name(mn), C_last(c), K(k), X0(x0), Y0(y0), bit_mrg(bit_mrg_), ptype(ptype), stype(stype),
  Type('C'), i_fmap("i_fmap"), o_Wsum("o_Wsum"), skip_cfg(a_skip),
  fifo_in("fifo_in"), fifo_in_2_syn("fifo_in_2_syn"), fifo_syn_2_out("fifo_syn_2_out"), fifo_out_2_act("fifo_out_2_act"){
  cout << "debug: Layer/Layer: pre" << endl;
  int x0_syn, y0_syn;
  if(en_zero_padding_in){
    x0_syn = x0 + i - 1;
    y0_syn = y0 + j - 1;
  }
  else{
    x0_syn = x0;
    y0_syn = y0;
  }
  X = (x0_syn - i)/stride + 1;
  Y = (y0_syn - j)/stride + 1;
  switch(stype){
    case st_ext:
      C = C_last + a_skip.sum_1();
      C_skip = a_skip.sum_1();
      K_skip = a_skip.sum();
      break;
    case st_par:
      C = C_last;
      C_skip = a_skip.sum();
      K_skip = a_skip.sum();
      break;
    case st_deg:
      C = C_last + a_skip.sum();
      C_skip = a_skip.sum();
      K_skip = 0;
      break;
    default:
      C = C_last;
      C_skip = 0;
      K_skip = 0;
  }

  if((ptype == Parallel_type::pt_2YS || ptype == Parallel_type::pt_2XYS) && stride != 2){
    cout << "Error: Layer::Layer(): Unsupported config!" << endl;
    exit(9487);
  }
  padding_in = new Padding_in("padding_in", C, i, j, x0, y0, en_zero_padding_in, ptype);
  synapse = new Synapse("synapse", C, K, i, j, y0_syn, en_DW, bit_mrg, ptype);
  padding_out = new Padding_out("padding_out", K, i, j, x0_syn, y0_syn, stride, bit_mrg, ptype);
  act = new Act("act", K, en_relu, ptype);
  
  P_total = synapse->P_total;
  num_RRAM = synapse->num_RRAM;
  
  int C_in_2_syn, K_syn_2_out;
  switch(ptype){
    case pt_2YS:
      mul_C = 1; C_in_2_syn = 2*C; K_syn_2_out = K;
      break;
    case pt_2XYS:
      mul_C = 1; C_in_2_syn = 4*C; K_syn_2_out = K;
      break;
    case pt_2Y:
      mul_C = 2; C_in_2_syn = 2*C; K_syn_2_out = 2*K;
      break;
    case pt_2XY:
      mul_C = 4; C_in_2_syn = 4*C; K_syn_2_out = 4*K;
      break;
    case pt_2Ym:
      mul_C = 2; C_in_2_syn = 2*C; K_syn_2_out = K;
      break;
    case pt_2XYm:
      mul_C = 4; C_in_2_syn = 4*C; K_syn_2_out = K;
      break;
    default:
      mul_C = 1; C_in_2_syn = C; K_syn_2_out = K;
  }

  i_fmap.init(mul_C*(C_last + C_skip));
  fifo_in.init(mul_C*C);
  for(int i_c = 0; i_c < mul_C*C; i_c++)
    padding_in->i_data[i_c](fifo_in[i_c]);

  fifo_in_2_syn.init(C_in_2_syn);
  for(int i_c = 0; i_c < C_in_2_syn; i_c++){
    padding_in->o_data[i_c](fifo_in_2_syn[i_c]);
    synapse->i_fmap[i_c](fifo_in_2_syn[i_c]);
  }
  
  fifo_syn_2_out.init(K_syn_2_out);
  for(int i_k = 0; i_k < K_syn_2_out; i_k++){
    synapse->o_Wsum[i_k](fifo_syn_2_out[i_k]);
    padding_out->i_data[i_k](fifo_syn_2_out[i_k]);
  }

  fifo_out_2_act.init(K_syn_2_out);
  o_Wsum.init(K_syn_2_out + mul_C*K_skip);
  for(int i_k = 0; i_k < K_syn_2_out; i_k++){
    padding_out->o_data[i_k](fifo_out_2_act[i_k]);
    act->i_data[i_k](fifo_out_2_act[i_k]);
    act->o_data[i_k](o_Wsum[i_k]);
  }

  fifo_skips = new sc_fifo< sc_int<IN_WIDTH> > *[mul_C*K_skip];
  if(i == 3){
    if(en_zero_padding_in){
      if(mul_C == 2)
        skip_L = y0/2 + 6;//2 + CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE;
      else if(mul_C == 4)
        skip_L = y0/2 + 6;//2 + CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE;
      else
        skip_L = y0 + 5;//1 + CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE;
    }
    else{
      if(mul_C == 2)
        skip_L = y0 + 5;//1 + CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE;
      else if(mul_C == 4)
        skip_L = y0/2 + 5;//1 + CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE;
      else
        skip_L = y0*2 + 6;//2 + CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE;
    }
  }
  else if(i == 1)
    skip_L = 4;//CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE;
  else
    skip_L = -1;

  for(int i_k = 0; i_k < mul_C*K_skip; i_k++)
    fifo_skips[i_k] = new sc_fifo< sc_int<IN_WIDTH> >(skip_L);

  SC_THREAD(data_transfer_in);
  if(stype == st_ext || stype == st_par)
    SC_THREAD(data_transfer_out);
  cout << "debug: Layer/Layer: pos" << endl;
}

Layer::Layer(const sc_module_name &mn, int c, int x0, int y0, int bit_mrg_, char pool_type)
  : sc_module(mn), name(mn), C(c), K(c), X0(x0), Y0(y0), X(1), Y(1), C_last(c), C_skip(0), K_skip(0), mul_C(1), bit_mrg(bit_mrg_), ptype(Parallel_type::none),
  Type(pool_type), i_fmap("i_fmap", c), o_Wsum("o_Wsum", c){
  cout << "debug: Layer/Layer: pre" << endl;
  switch(Type){
    case 'M':
      SC_THREAD(maxpooling);
      break;
    case 'A':
      SC_THREAD(avgpooling);
      break;
    default:
      cout << "Layer::Layer: Invalid pooling type!" << endl;
      exit(1);
  }
  P_total = X0 * Y0 * C;
  num_RRAM = 0;
  skip_L = -1;
  cout << "debug: Layer/Layer: pos" << endl;
}

Layer::~Layer(){
  if(Type == 'C'){
    for(int i_k = 0; i_k < mul_C*K_skip; i_k++)
      delete fifo_skips[i_k];
    delete [] fifo_skips;
    delete padding_in;
    delete synapse;
    delete padding_out;
    delete act;
  }
}

void Layer::map_filters(int ****Weights, int *bias, int *scale){
  if(Type == 'C'){
    //cout << "debug: Layer/map_filters: synapse" << endl;
    synapse->map_filters(Weights);
    //cout << "debug: Layer/map_filters: scale" << endl;
    padding_out->map_scale(scale); // {M_m_mul, M_m_sft}
    //cout << "debug: Layer/map_filters: act" << endl;
    act->map_bias(bias, scale + 2); // {M_b_mul, M_b_sft}
  }
}

void Layer::fetch_cycle_count(){
  if(Type == 'C'){
    cycle_cal_v = padding_out->cycle_cal_v;
    cycle_cal_inv = padding_out->cycle_cal_inv;
  }
}

void Layer::fifo_reset(){
  //cout << "debug: Layer/fifo_reset: pre" << endl;
  for(int i_k = 0; i_k < mul_C*K_skip; i_k++){
    for(int i_d = 0; i_d < CLK_CYCLE + PADDING_IN_CYCLE + PADDING_OUT_CYCLE + ACT_CYCLE; i_d++){
      fifo_skips[i_k]->write(0);
      //cout << "fifo_skips[" << i_d << "]->write(0)" << endl;
    }
    //fifo_skips[i_k]->write(0);
    //cout << "fifo[" << i_k << "].write(0);" << endl;
  }
  //wait(CLK_CYCLE, SC_NS);
  //cout << "debug: Layer/fifo_reset: pos" << endl;
}

void Layer::data_transfer_in(){
  //cout << "debug: Layer/data_transfer: pre" << endl;
  //fifo_reset();
  sc_int<IN_WIDTH> data_tmp;
  switch(stype){
    case st_ext:
      while(true){
        //cout << "debug: Layer/data_transfer_in: st_ext pre" << endl;
        for(int i_c = 0; i_c < mul_C*C_last; i_c++){
          data_tmp = i_fmap[i_c]->read();
          fifo_in[i_c].write(data_tmp);
          fifo_skips[mul_C*C_skip + i_c]->write(data_tmp);
        }
        for(int i_c = 0; i_c < mul_C*C_skip; i_c++){
          data_tmp = i_fmap[mul_C*C_last + i_c]->read();
          fifo_in[mul_C*C_last + i_c].write(data_tmp);
          fifo_skips[i_c]->write(data_tmp);
        }
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer_in: st_ext pos" << endl;
      }
      break;
    case st_par:
      while(true){
        //cout << "debug: Layer/data_transfer: st_par pre" << endl;
        for(int i_c = 0; i_c < mul_C*C_last; i_c++){
          data_tmp = i_fmap[i_c]->read();
          fifo_in[i_c].write(data_tmp);
        }
        for(int i_c = 0; i_c < mul_C*C_skip; i_c++){
          data_tmp = i_fmap[mul_C*C_last + i_c]->read();
          fifo_skips[i_c]->write(data_tmp);
        }
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer: st_par pos" << endl;
      }
      break;
    case st_deg:
      while(true){
        //cout << "debug: Layer/data_transfer: st_deg pre" << endl;
        for(int i_c = 0; i_c < mul_C*C; i_c++){
          data_tmp = i_fmap[i_c]->read();
          fifo_in[i_c].write(data_tmp);
        }
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer: st_deg pos" << endl;
      }
      break;
    default:
      while(true){
        //cout << "debug: Layer/data_transfer: st_none pre" << endl;
        for(int i_c = 0; i_c < mul_C*C; i_c++){
          data_tmp = i_fmap[i_c]->read();
          fifo_in[i_c].write(data_tmp);
        }
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer: st_none pos" << endl;
      }
  }
}

void Layer::data_transfer_out(){
  //cout << "debug: Layer/data_transfer: pre" << endl;
  //fifo_reset();
  sc_int<IN_WIDTH> data_tmp;
  switch(stype){
    case st_ext:
      while(true){
        //cout << "debug: Layer/data_transfer: st_ext pre" << endl;
        for(int i_k = 0; i_k < mul_C*K_skip; i_k++){
          data_tmp = fifo_skips[i_k]->read();
          o_Wsum[mul_C*K + i_k]->write(data_tmp);
        }
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer: st_ext pos" << endl;
      }
      break;
    case st_par:
      while(true){
        //cout << "debug: Layer/data_transfer: st_par pre" << endl;
        for(int i_k = 0; i_k < mul_C*K_skip; i_k++){
          data_tmp = fifo_skips[i_k]->read();
          o_Wsum[mul_C*K + i_k]->write(data_tmp);
        }
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer: st_par pos" << endl;
      }
      break;
    case st_deg:
      while(true){
        //cout << "debug: Layer/data_transfer: st_deg pre" << endl;
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer: st_deg pos" << endl;
      }
      break;
    default:
      while(true){
        //cout << "debug: Layer/data_transfer: st_none pre" << endl;
        wait(0, SC_NS);
        //cout << "debug: Layer/data_transfer: st_none pos" << endl;
      }
  }
}

void Layer::maxpooling(){
  //cout << "debug: Layer/maxpooling: pre" << endl;
  cycle_cal_v = 0;
  cycle_cal_inv = 0;
  while(true){
    long long int num_max[C];
    fill_n(num_max, C, LLONG_MIN);
    for(int i_x0 = 0; i_x0 < X0; i_x0++)
      for(int i_y0 = 0; i_y0 < Y0; i_y0++){
        //i_rst->read();
        for(int i_c = 0; i_c < C; i_c++){
          sc_int<IN_WIDTH> tmp_data = i_fmap[i_c]->read();
          if(tmp_data > num_max[i_c])
            num_max[i_c] = tmp_data;
        }
        cycle_cal_inv++;
        wait(POOL_CYCLE, SC_NS);
      }
    for(int i_c = 0; i_c < C; i_c++)
      o_Wsum[i_c]->write(num_max[i_c]);
    cycle_cal_inv--;
    cycle_cal_v++;
  }
}

void Layer::avgpooling(){
  //cout << "debug: Layer/avgpooling: pre" << endl;
  cycle_cal_v = 0;
  cycle_cal_inv = 0;
  const long long int max_psum = (1LL << (IN_WIDTH + bit_mrg - 1)) - 1;
  const long long int min_psum = -(1LL << (IN_WIDTH + bit_mrg - 1));
  while(true){
    sc_int<16> psum[C];
    fill_n(psum, C, 0);
    for(int i_x0 = 0; i_x0 < X0; i_x0++)
      for(int i_y0 = 0; i_y0 < Y0; i_y0++){
        for(int i_c = 0; i_c < C; i_c++){
          psum[i_c] += i_fmap[i_c]->read();
          if(psum[i_c] > max_psum)
            psum[i_c] = max_psum;
          else if(psum[i_c] < min_psum)
            psum[i_c] = min_psum;
          ////cout << "debug: Layer/avgpooling: " << i_x0 << ", " << i_y0 << ", " << i_c << " = " << tmp << endl;
        }
        cycle_cal_inv++;
        wait(POOL_CYCLE, SC_NS);
      }
    for(int i_c = 0; i_c < C; i_c++){
      psum[i_c] /= X0 * Y0;
      o_Wsum[i_c]->write(psum[i_c]);//e
    }
    cycle_cal_inv--;
    cycle_cal_v++;
  }
}
