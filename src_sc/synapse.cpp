#include "synapse.h"

sc_vector< sc_fifo< sc_int<IN_WIDTH> > > Synapse::v_null("v_null", 0);

Synapse::Synapse(const sc_module_name &mn, int c, int k, int i, int j, int y0, bool en_DW, int bit_mrg_, Parallel_type ptype)
  : sc_module(mn), I(i), J(j), C(c), K(k), Y0(y0), en_DW(en_DW),
  P_total(0), num_RRAM(0),
  max_psum((1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1)) - 1),
  min_psum(-(1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1))),
  max_out((1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1)) - 1),
  min_out(-(1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1))),
  bit_mrg(bit_mrg_),
  i_fmap("i_fmap"), o_Wsum("o_Wsum"){
  if(ptype != Parallel_type::none && (I != J || I != 1 && I != 2 && I != 3)){
    cout << "Error: Synapse::Synapse(): Unsupported config!" << endl;
    exit(9487);
  }
  //cout << "debug: Synapse/Synapse: pre" << endl;
  switch(ptype){
    case pt_2YS: case pt_2Ym:
      num.iC = 2*C; num.oK = K;
      num.fiI = I-1; num.fiC = 2*C; num.fiL = (Y0+1)/2;
      num.fjI = I; num.fjJ = (J-1)/2; num.fjC = 2*C;
      num.pI = I; num.pJ = J; num.pC = C; num.pK = K;
      build();
      SC_THREAD(data_transfer_2YS);
      SC_THREAD(partial_sum);
      break;
    case pt_2XYS: case pt_2XYm:
      num.iC = 4*C; num.oK = K;
      num.fiI = (I-1)/2; num.fiC = 4*C; num.fiL = (Y0+1)/2;
      num.fjI = I; num.fjJ = (J-1)/2; num.fjC = 4*C;
      num.pI = I; num.pJ = J; num.pC = C; num.pK = K;
      build();
      SC_THREAD(data_transfer_2XYS);
      SC_THREAD(partial_sum);
      break;
    case pt_2Y:
      num.iC = 2*C; num.oK = 2*K;
      num.fiI = I-1; num.fiC = 2*C; num.fiL = (Y0+1)/2;
      num.fjI = I; num.fjJ = (J-1)/2; num.fjC = 2*C;
      num.pI = 2*I; num.pJ = J; num.pC = C; num.pK = K;
      build();
      SC_THREAD(data_transfer_2Y);
      SC_THREAD(partial_sum_2Y);
      break;
    case pt_2XY:
      num.iC = 4*C; num.oK = 4*K;
      num.fiI = (I-1)/2; num.fiC = 4*C; num.fiL = (Y0+1)/2;
      num.fjI = I; num.fjJ = (J-1)/2; num.fjC = 4*C;
      num.pI = 2*I; num.pJ = 2*J; num.pC = C; num.pK = K;
      build();
      SC_THREAD(data_transfer_2XY);
      SC_THREAD(partial_sum_2XY);
      break;
    default:
      num.iC = C; num.oK = K;
      num.fiI = I-1; num.fiC = C; num.fiL = Y0;
      num.fjI = I; num.fjJ = J-1; num.fjC = C;
      num.pI = I; num.pJ = J; num.pC = C; num.pK = K;
      build();
      SC_THREAD(data_transfer);
      SC_THREAD(partial_sum);
  }
  //SC_THREAD(debug);//debug
  //cout << "debug: Synapse/Synapse: pos" << endl;
}

void Synapse::build(){
  i_fmap.init(num.iC);
  o_Wsum.init(num.oK);
  fifo_is = new sc_fifo< sc_int<IN_WIDTH> > **[num.fiI]; // (I, C)
  for(int i_i = 0; i_i < num.fiI; i_i++){
    fifo_is[i_i] = new sc_fifo< sc_int<IN_WIDTH> > *[num.fiC];
    for(int i_c = 0; i_c < num.fiC; i_c++)
      fifo_is[i_i][i_c] = new sc_fifo< sc_int<IN_WIDTH> >(num.fiL);
  }
  fifo_js = new sc_fifo< sc_int<IN_WIDTH> > ***[num.fjI]; // (I, J, C)
  for(int i_i = 0; i_i < num.fjI; i_i++){
    fifo_js[i_i] = new sc_fifo< sc_int<IN_WIDTH> > **[num.fjJ];
    for(int i_j = 0; i_j < num.fjJ; i_j++){
      fifo_js[i_i][i_j] = new sc_fifo< sc_int<IN_WIDTH> > *[num.fjC];
      for(int i_c = 0; i_c < num.fjC; i_c++)
        fifo_js[i_i][i_j][i_c] = new sc_fifo< sc_int<IN_WIDTH> >(1);
    }
  }
  pe_array_CKs = new PE_array_CK **[num.pI]; // (I, J)
  vector_pe_ins = new sc_vector< sc_fifo< sc_int<IN_WIDTH> > > *[num.pI]; // (I, J)
  vector_pe_outs = new sc_vector< sc_fifo< sc_int<PSUM_WIDTH> > > *[num.pI]; // (I, J)
  for(int i_i = 0; i_i < num.pI; i_i++){
    pe_array_CKs[i_i] = new PE_array_CK *[num.pJ];
    vector_pe_ins[i_i] = new sc_vector< sc_fifo< sc_int<IN_WIDTH> > >[num.pJ];
    vector_pe_outs[i_i] = new sc_vector< sc_fifo< sc_int<PSUM_WIDTH> > >[num.pJ];
    for(int i_j = 0; i_j < num.pJ; i_j++){
      char str_[20];
      sprintf(str_, "pe_array_CK_%d,%d", i_i, i_j);
      pe_array_CKs[i_i][i_j] = new PE_array_CK(str_, num.pC, num.pK, en_DW, bit_mrg);
      /* ----- HW counter ----- */
      num_RRAM++;
      P_total += pe_array_CKs[i_i][i_j]->P_total;

      vector_pe_ins[i_i][i_j].init(num.pC);
      vector_pe_outs[i_i][i_j].init(num.pK);
      for(int i_c = 0; i_c < num.pC; i_c++)
        pe_array_CKs[i_i][i_j]->i_fmap[i_c](vector_pe_ins[i_i][i_j][i_c]);
      for(int i_k = 0; i_k < num.pK; i_k++)
        pe_array_CKs[i_i][i_j]->o_Wsum[i_k](vector_pe_outs[i_i][i_j][i_k]);
    }
  }
}
/*
void Synapse::debug(){
  int i = 0;
  while(true){
    cout << "Synapse::debug: 1" << endl;
    cout << "i_rst.read(): pre" << endl;
    bool b = i_rst->read();
    cout << "i_rst.read(): " << b << endl;
    if(!b){
      for(int i_c = 0; i_c < C; i_c++){
        sc_int<IN_WIDTH> data_tmp = i_fmap[i_c]->read();
        cout << "i=" << i << "; i_c=" << i_c << "; data=" << data_tmp << endl;
      }
      for(int i_k = 0; i_k < K; i_k++){
        o_Wsum[i_k]->write(i*10+i_k);
      }
      i += 1;
    }
    cout << "Synapse::debug: 2" << endl;
    wait(CLK_CYCLE, SC_NS);
  }
}
*/
Synapse::~Synapse(){
  for(int i_i = 0; i_i < num.fiI; i_i++){
    for(int i_c = 0; i_c < num.fiC; i_c++)
      delete fifo_is[i_i][i_c];
    delete [] fifo_is[i_i];
  }
  delete [] fifo_is;
  for(int i_i = 0; i_i < num.fjI; i_i++){
    for(int i_j = 0; i_j < num.fjJ; i_j++){
      for(int i_c = 0; i_c < num.fjC; i_c++)
        delete fifo_js[i_i][i_j][i_c];
      delete [] fifo_js[i_i][i_j];
    }
    delete [] fifo_js[i_i];
  }
  delete [] fifo_js;
  for(int i_i = 0; i_i < num.pI; i_i++){
    for(int i_j = 0; i_j < num.pJ; i_j++)
      delete pe_array_CKs[i_i][i_j];
    delete [] pe_array_CKs[i_i];
    delete [] vector_pe_ins[i_i];
    delete [] vector_pe_outs[i_i];
  }
  delete [] pe_array_CKs;
  delete [] vector_pe_ins;
  delete [] vector_pe_outs;
}

void Synapse::map_filters(int**** Filter){
  for(int i_i = 0; i_i < num.pI; i_i++)
    for(int i_j = 0; i_j < num.pJ; i_j++){
      //cout << "debug: Synapse/map_filters: " << i_i << ", " << i_j << endl;
      pe_array_CKs[i_i][i_j]->map_filters(Filter[i_i % I][i_j % J]);
    }
}

void Synapse::fifo_reset(){
  //cout << "debug: Synapse/fifo_reset: pre" << endl;
  for(int i_i = 0; i_i < num.fiI; i_i++)
    for(int i_c = 0; i_c < num.fiC; i_c++){
      for(int i_y0 = 0; i_y0 < num.fiL; i_y0++)
        fifo_is[i_i][i_c]->write(0);
      //cout << "fifo_is[" << i_i << "][" << i_c << "].write(0);" << endl;
    }
  for(int i_i = 0; i_i < num.fjI; i_i++)
    for(int i_j = 0; i_j < num.fjJ; i_j++)
      for(int i_c = 0; i_c < num.fjC; i_c++){
        fifo_js[i_i][i_j][i_c]->write(0);
        //cout << "fifo_js[" << i_i << "][" << i_j << "][" << i_c << "].write(0);" << endl;
      }
  //cout << "debug: Synapse/fifo_reset: mid_pos" << endl;
  //wait(CLK_CYCLE, SC_NS);
  //cout << "debug: Synapse/fifo_reset: pos" << endl;
}
/**/
void Synapse::data_transfer(){
  fifo_reset();
  while(true){
    //cout << "debug: Synapse/data_transfer: pre" << endl;
    for(int i_i = 0; i_i < I; i_i++){
      for(int i_j = 0; i_j < J-1; i_j++){
        if(i_j == 0)
          data_transfer_C(fifo_js[i_i][0], vector_pe_ins[i_i][0]); // f -> v
        else
          data_transfer_C(fifo_js[i_i][i_j], fifo_js[i_i][i_j-1], vector_pe_ins[i_i][i_j]); // f -> f, v
      }
      /*
      if(i_i == 0)
        data_transfer_C(fifo_is[0], fifo_js[0][J-2], vector_pe_ins[0][J-1]);
      else if(i_i == I-1)
        data_transfer_C(i_fmap, fifo_is[I-2], fifo_js[I-1][J-2], vector_pe_ins[I-1][J-1]);
      else
        data_transfer_C(fifo_is[i_i], fifo_is[i_i-1], fifo_js[i_i][J-2], vector_pe_ins[i_i][J-1]);
      */
      if(I == 1){
        if(J == 1)
          data_transfer_C(0, i_fmap, vector_pe_ins[0][0]); // p -> v
        else
          data_transfer_C(0, i_fmap, fifo_js[0][J-2], vector_pe_ins[0][0]); // p -> f, v
      }
      else if(i_i == 0){
        if(J == 1)
          data_transfer_C(fifo_is[0], vector_pe_ins[0][0]); // f -> v
        else
          data_transfer_C(fifo_is[0], fifo_js[0][J-2], vector_pe_ins[0][J-1]); // f -> f, v
      }
      else if(i_i == I-1){
        if(J == 1)
          data_transfer_C(0, i_fmap, fifo_is[I-2], vector_pe_ins[I-1][0]); // p -> f, v
        else
          data_transfer_C(0, i_fmap, fifo_is[I-2], fifo_js[I-1][J-2], vector_pe_ins[I-1][J-1]); // p -> f, f, v
      }
      else
        data_transfer_C(fifo_is[i_i], fifo_is[i_i-1], fifo_js[i_i][J-2], vector_pe_ins[i_i][J-1]); // f -> f, f, v
    }
    //cout << "debug: Synapse/data_transfer: pos" << endl;
    //wait(CLK_CYCLE, SC_NS);
  }
}

void Synapse::data_transfer_2YS(){
  fifo_reset();
  if(I == 3)
    while(true){
      //cout << "debug: Synapse/data_transfer_2YS: pre" << endl;
      data_transfer_C(fifo_js[0][0],   vector_pe_ins[0][0]); // f -> v
      data_transfer_C(fifo_js[0][0]+C, vector_pe_ins[0][1]); // f -> v
      data_transfer_C(fifo_js[1][0],   vector_pe_ins[1][0]); // f -> v
      data_transfer_C(fifo_js[1][0]+C, vector_pe_ins[1][1]); // f -> v
      data_transfer_C(fifo_js[2][0],   vector_pe_ins[2][0]); // f -> v
      data_transfer_C(fifo_js[2][0]+C, vector_pe_ins[2][1]); // f -> v
      data_transfer_C(fifo_is[0],   fifo_js[0][0],               vector_pe_ins[0][2]); // f -> f, v
      data_transfer_C(fifo_is[0]+C, fifo_js[0][0]+C); // f -> f
      data_transfer_C(fifo_is[1],   fifo_js[1][0],   fifo_is[0], vector_pe_ins[1][2]); // f -> f, f, v
      data_transfer_C(fifo_is[1]+C, fifo_js[1][0]+C, fifo_is[0]+C); // f -> f, f
      data_transfer_C(0, i_fmap,    fifo_js[2][0],   fifo_is[1], vector_pe_ins[2][2]); // p -> f, f, v
      data_transfer_C(C, i_fmap,    fifo_js[2][0]+C, fifo_is[1]+C); // p -> f, f
      //cout << "debug: Synapse/data_transfer_2YS: pos" << endl;
    }
  else if(I == 2)
    while(true){
      //cout << "debug: Synapse/data_transfer_2YS: pre" << endl;
      data_transfer_C(fifo_is[0],              vector_pe_ins[0][0]); // f -> v
      data_transfer_C(fifo_is[0]+C,            vector_pe_ins[0][1]); // f -> v
      data_transfer_C(0, i_fmap, fifo_is[0],   vector_pe_ins[1][0]); // p -> f, v
      data_transfer_C(C, i_fmap, fifo_is[0]+C, vector_pe_ins[1][1]); // p -> f, v
      //cout << "debug: Synapse/data_transfer_2YS: pos" << endl;
    }
  else if(I == 1)
    while(true){
      //cout << "debug: Synapse/data_transfer_2YS: pre" << endl;
      data_transfer_C(0, i_fmap, vector_pe_ins[0][0]); // p -> v
      data_transfer_C(C, i_fmap); // p -> x
      //cout << "debug: Synapse/data_transfer_2YS: pos" << endl;
    }
}

void Synapse::data_transfer_2XYS(){
  fifo_reset();
  if(I == 3)
    while(true){
      //cout << "debug: Synapse/data_transfer_2XYS: pre" << endl;
      data_transfer_C(fifo_js[0][0],   vector_pe_ins[0][0]); // f -> v
      data_transfer_C(fifo_js[0][0]+C, vector_pe_ins[0][1]); // f -> v
      data_transfer_C(fifo_js[1][0],   vector_pe_ins[1][0]); // f -> v
      data_transfer_C(fifo_js[1][0]+C, vector_pe_ins[1][1]); // f -> v
      data_transfer_C(fifo_js[2][0],   vector_pe_ins[2][0]); // f -> v
      data_transfer_C(fifo_js[2][0]+C, vector_pe_ins[2][1]); // f -> v
      data_transfer_C(fifo_is[0],     fifo_js[0][0],               vector_pe_ins[0][2]); // f -> f, v
      data_transfer_C(fifo_is[0]+1*C, fifo_js[0][0]+C); // f -> f
      data_transfer_C(fifo_is[0]+2*C, fifo_js[1][0],               vector_pe_ins[1][2]); // f -> f, v
      data_transfer_C(fifo_is[0]+3*C, fifo_js[1][0]+C); // f -> f
      data_transfer_C(  0, i_fmap,    fifo_js[2][0],   fifo_is[0], vector_pe_ins[2][2]); // p -> f, f, v
      data_transfer_C(1*C, i_fmap,    fifo_js[2][0]+C, fifo_is[0]+1*C); // p -> f, f
      data_transfer_C(2*C, i_fmap,                     fifo_is[0]+2*C); // p -> f
      data_transfer_C(3*C, i_fmap,                     fifo_is[0]+3*C); // p -> f
      //cout << "debug: Synapse/data_transfer_2XYS: pos" << endl;
    }
  else if(I == 2)
    while(true){
      //cout << "debug: Synapse/data_transfer_2XYS: pre" << endl;
      data_transfer_C(  0, i_fmap, vector_pe_ins[0][0]); // p -> v
      data_transfer_C(1*C, i_fmap, vector_pe_ins[0][1]); // p -> v
      data_transfer_C(2*C, i_fmap, vector_pe_ins[1][0]); // p -> v
      data_transfer_C(3*C, i_fmap, vector_pe_ins[1][1]); // p -> v
      //cout << "debug: Synapse/data_transfer_2XYS: pos" << endl;
    }
  else if(I == 1)
    while(true){
      //cout << "debug: Synapse/data_transfer_2XYS: pre" << endl;
      data_transfer_C(  0, i_fmap, vector_pe_ins[0][0]); // p -> v
      data_transfer_C(1*C, i_fmap); // p -> x
      data_transfer_C(2*C, i_fmap); // p -> x
      data_transfer_C(3*C, i_fmap); // p -> x
      //cout << "debug: Synapse/data_transfer_2XYS: pos" << endl;
    }
}

void Synapse::data_transfer_2Y(){
  fifo_reset();
  if(I == 3)
    while(true){
      //cout << "debug: Synapse/data_transfer_2Y: pre" << endl;
      data_transfer_C(fifo_js[0][0],   vector_pe_ins[0][0]); // f -> v
      data_transfer_C(fifo_js[0][0]+C, vector_pe_ins[0][1], vector_pe_ins[1][0]); // f -> v2
      data_transfer_C(fifo_js[1][0],   vector_pe_ins[2][0]); // f -> v
      data_transfer_C(fifo_js[1][0]+C, vector_pe_ins[2][1], vector_pe_ins[3][0]); // f -> v2
      data_transfer_C(fifo_js[2][0],   vector_pe_ins[4][0]); // f -> v
      data_transfer_C(fifo_js[2][0]+C, vector_pe_ins[4][1], vector_pe_ins[5][0]); // f -> v2
      data_transfer_C(fifo_is[0],   fifo_js[0][0],                 vector_pe_ins[0][2], vector_pe_ins[1][1]); // f -> f, v2
      data_transfer_C(fifo_is[0]+C, fifo_js[0][0]+C,               vector_pe_ins[1][2]); // f -> f, v
      data_transfer_C(fifo_is[1],   fifo_js[1][0],   fifo_is[0],   vector_pe_ins[2][2], vector_pe_ins[3][1]); // f -> f, f, v2
      data_transfer_C(fifo_is[1]+C, fifo_js[1][0]+C, fifo_is[0]+C, vector_pe_ins[3][2]); // f -> f, f, v
      data_transfer_C(0, i_fmap,    fifo_js[2][0],   fifo_is[1],   vector_pe_ins[4][2], vector_pe_ins[5][1]); // p -> f, f, v2
      data_transfer_C(C, i_fmap,    fifo_js[2][0]+C, fifo_is[1]+C, vector_pe_ins[5][2]); // p -> f, f, v
      //cout << "debug: Synapse/data_transfer_2Y: pos" << endl;
    }
  else if(I == 1)
    while(true){
      //cout << "debug: Synapse/data_transfer_2Y: pre" << endl;
      data_transfer_C(0, i_fmap, vector_pe_ins[0][0]); // p -> v
      data_transfer_C(C, i_fmap, vector_pe_ins[1][0]); // p -> v
      //cout << "debug: Synapse/data_transfer_2Y: pos" << endl;
    }
}

void Synapse::data_transfer_2XY(){
  fifo_reset();
  if(I == 3)
    while(true){
      //cout << "debug: Synapse/data_transfer_2XY: pre" << endl;
      /* oridinal 2 blocks
      data_transfer_C(fifo_js[0][0],   vector_pe_ins[0][0]); // f -> v
      data_transfer_C(fifo_js[0][0]+C, vector_pe_ins[0][2], vector_pe_ins[1][0]); // f -> v
      data_transfer_C(fifo_js[1][0],   vector_pe_ins[2][0]); // f -> v
      data_transfer_C(fifo_js[1][0]+C, vector_pe_ins[2][2], vector_pe_ins[3][0]); // f -> v
      data_transfer_C(fifo_js[2][0],   vector_pe_ins[4][0]); // f -> v
      data_transfer_C(fifo_js[2][0]+C, vector_pe_ins[4][2], vector_pe_ins[5][0]); // f -> v
      data_transfer_C(fifo_is[0],     fifo_js[0][0],                   vector_pe_ins[0][4], vector_pe_ins[1][2]); // f -> f, v
      data_transfer_C(fifo_is[0]+1*C, fifo_js[0][0]+C,                 vector_pe_ins[1][4]); // f -> f
      data_transfer_C(fifo_is[0]+2*C, fifo_js[1][0],                   vector_pe_ins[2][4], vector_pe_ins[3][2]); // f -> f, f, v
      data_transfer_C(fifo_is[0]+3*C, fifo_js[1][0]+C,                 vector_pe_ins[3][4]); // f -> f, f, v
      data_transfer_C(0,   i_fmap,    fifo_js[2][0],   fifo_is[0],     vector_pe_ins[4][4], vector_pe_ins[5][2]); // p -> f, f, v
      data_transfer_C(1*C, i_fmap,    fifo_js[2][0]+C, fifo_is[0]+1*C, vector_pe_ins[5][4]); // p -> f, f
      data_transfer_C(2*C, i_fmap,                     fifo_is[0]+2*C)
      data_transfer_C(3*C, i_fmap,                     fifo_is[0]+3*C)

      data_transfer_C(fifo_js[0][0]+2*C, vector_pe_ins[0][1]); // f -> v
      data_transfer_C(fifo_js[0][0]+3*C, vector_pe_ins[0][3], vector_pe_ins[1][1]); // f -> v
      data_transfer_C(fifo_js[1][0]+2*C, vector_pe_ins[2][1]); // f -> v
      data_transfer_C(fifo_js[1][0]+3*C, vector_pe_ins[2][3], vector_pe_ins[3][1]); // f -> v
      data_transfer_C(fifo_js[2][0]+2*C, vector_pe_ins[4][1]); // f -> v
      data_transfer_C(fifo_js[2][0]+3*C, vector_pe_ins[4][3], vector_pe_ins[5][1]); // f -> v
      data_transfer_C(fifo_is[0]+2*C, fifo_js[0][0]+2*C,                 vector_pe_ins[0][5], vector_pe_ins[1][3]); // f -> f, v
      data_transfer_C(fifo_is[0]+3*C, fifo_js[0][0]+3*C,                 vector_pe_ins[1][5]); // f -> f
      data_transfer_C(0,   i_fmap,    fifo_js[1][0]+2*C, fifo_is[0],     vector_pe_ins[2][5], vector_pe_ins[3][3]); // f -> f, f, v
      data_transfer_C(1*C, i_fmap,    fifo_js[1][0]+3*C, fifo_is[0]+1*C, vector_pe_ins[3][5]); // f -> f, f, v
      data_transfer_C(2*C, i_fmap,    fifo_js[2][0]+2*C, fifo_is[0]+2*C, vector_pe_ins[4][5], vector_pe_ins[5][3]); // p -> f, f, v
      data_transfer_C(3*C, i_fmap,    fifo_js[2][0]+3*C, fifo_is[0]+3*C, vector_pe_ins[5][5]); // p -> f, f
      */
      data_transfer_C(fifo_js[0][0],     vector_pe_ins[0][0]); // f -> v
      data_transfer_C(fifo_js[0][0]+1*C, vector_pe_ins[0][2], vector_pe_ins[1][0]); // f -> v2
      data_transfer_C(fifo_js[0][0]+2*C, vector_pe_ins[0][1]); // f -> v
      data_transfer_C(fifo_js[0][0]+3*C, vector_pe_ins[0][3], vector_pe_ins[1][1]); // f -> v2
      data_transfer_C(fifo_js[1][0],     vector_pe_ins[2][0]); // f -> v
      data_transfer_C(fifo_js[1][0]+1*C, vector_pe_ins[2][2], vector_pe_ins[3][0]); // f -> v2
      data_transfer_C(fifo_js[1][0]+2*C, vector_pe_ins[2][1]); // f -> v
      data_transfer_C(fifo_js[1][0]+3*C, vector_pe_ins[2][3], vector_pe_ins[3][1]); // f -> v2
      data_transfer_C(fifo_js[2][0],     vector_pe_ins[4][0]); // f -> v
      data_transfer_C(fifo_js[2][0]+1*C, vector_pe_ins[4][2], vector_pe_ins[5][0]); // f -> v2
      data_transfer_C(fifo_js[2][0]+2*C, vector_pe_ins[4][1]); // f -> v
      data_transfer_C(fifo_js[2][0]+3*C, vector_pe_ins[4][3], vector_pe_ins[5][1]); // f -> v2
      data_transfer_C(fifo_is[0],     fifo_js[0][0],                                        vector_pe_ins[0][4], vector_pe_ins[1][2]); // f -> f, v2
      data_transfer_C(fifo_is[0]+1*C, fifo_js[0][0]+1*C,                                    vector_pe_ins[1][4]); // f -> f, v
      data_transfer_C(fifo_is[0]+2*C, fifo_js[1][0],     fifo_js[0][0]+2*C,                 vector_pe_ins[2][4], vector_pe_ins[3][2], vector_pe_ins[0][5], vector_pe_ins[1][3]); // f -> f, f, v4
      data_transfer_C(fifo_is[0]+3*C, fifo_js[1][0]+1*C, fifo_js[0][0]+3*C,                 vector_pe_ins[3][4], vector_pe_ins[1][5]); // f -> f, f, v2
      data_transfer_C(0,   i_fmap,    fifo_js[2][0],     fifo_js[1][0]+2*C, fifo_is[0],     vector_pe_ins[4][4], vector_pe_ins[5][2], vector_pe_ins[2][5], vector_pe_ins[3][3]); // p -> f, f, f, v4
      data_transfer_C(1*C, i_fmap,    fifo_js[2][0]+1*C, fifo_js[1][0]+3*C, fifo_is[0]+1*C, vector_pe_ins[5][4], vector_pe_ins[3][5]); // p -> f, f, f, v2
      data_transfer_C(2*C, i_fmap,    fifo_js[2][0]+2*C,                    fifo_is[0]+2*C, vector_pe_ins[4][5], vector_pe_ins[5][3]); // p -> f, f, v2
      data_transfer_C(3*C, i_fmap,    fifo_js[2][0]+3*C,                    fifo_is[0]+3*C, vector_pe_ins[5][5]); // p -> f, f, v
      //cout << "debug: Synapse/data_transfer_2XY: pos" << endl;
    }
  else if(I == 1)
    while(true){
      //cout << "debug: Synapse/data_transfer_2XY: pre" << endl;
      data_transfer_C(0,   i_fmap, vector_pe_ins[0][0]); // p -> v
      data_transfer_C(1*C, i_fmap, vector_pe_ins[1][0]); // p -> v
      data_transfer_C(2*C, i_fmap, vector_pe_ins[0][1]); // p -> v
      data_transfer_C(3*C, i_fmap, vector_pe_ins[1][1]); // p -> v
      //cout << "debug: Synapse/data_transfer_2XY: pos" << endl;
    }
}


void Synapse::data_transfer_D(){
  sc_int<IN_WIDTH> data_tmp;
  while(true){
    fifo_reset();
    //cout << "debug: Synapse/data_transfer_D: pre" << endl;
    wait(CLK_CYCLE, SC_NS);
    for(int i_c = 0; i_c < num.iC; i_c++)
      data_tmp = i_fmap[i_c]->read();
    for(int i_c = 0; i_c < num.pC; i_c++)
      for(int i_i = 0; i_i < num.pI; i_i++)
        for(int i_j = 0; i_j < num.pJ; i_j++)
          vector_pe_ins[i_i][i_j][i_c].write(1);
    //cout << "debug: Synapse/data_transfer_D: pos" << endl;
  }
}

void Synapse::partial_sum(){
  while(true){
    //cout << "debug: Synapse/partial_sum: pre" << endl;
    for(int i_k = 0; i_k < K; i_k++){
      sc_int<64> psum = 0;
      for(int i_i = 0; i_i < I; i_i++){
        for(int i_j = 0; i_j < J; i_j++){
          psum += vector_pe_outs[i_i][i_j][i_k].read();
          if(psum > max_psum)
            psum = max_psum;
          else if(psum < min_psum)
            psum = min_psum;
        }
      }
      if(psum > max_out)
        o_Wsum[i_k]->write(max_out);
      else if(psum < min_out)
        o_Wsum[i_k]->write(min_out);
      else
        o_Wsum[i_k]->write(psum);
    }
    //cout << "debug: Synapse/partial_sum: pos" << endl;
  }
}

void Synapse::partial_sum_2Y(){
  while(true){
    //cout << "debug: Synapse/partial_sum_2Y: pre" << endl;
    for(int j_i = 0; j_i < 2; j_i++)
      for(int i_k = 0; i_k < K; i_k++){
        sc_int<64> psum = 0;
        for(int i_i = 0; i_i < I; i_i++){
          for(int i_j = 0; i_j < J; i_j++){
            psum += vector_pe_outs[2*i_i + j_i][i_j][i_k].read();
            if(psum > max_psum)
              psum = max_psum;
            else if(psum < min_psum)
              psum = min_psum;
          }
        }
        if(psum > max_out)
          o_Wsum[K*j_i + i_k]->write(max_out);
        else if(psum < min_out)
          o_Wsum[K*j_i + i_k]->write(min_out);
        else
          o_Wsum[K*j_i + i_k]->write(psum);
      }
    //cout << "debug: Synapse/partial_sum_2Y: pos" << endl;
  }
}

void Synapse::partial_sum_2XY(){
  while(true){
    //cout << "debug: Synapse/partial_sum_2XY: pre" << endl;
    for(int j_j = 0; j_j < 2; j_j++)
      for(int j_i = 0; j_i < 2; j_i++)
        for(int i_k = 0; i_k < K; i_k++){
          sc_int<64> psum = 0;
          for(int i_i = 0; i_i < I; i_i++){
            for(int i_j = 0; i_j < J; i_j++){
              psum += vector_pe_outs[2*i_i + j_i][2*i_j + j_j][i_k].read();
              if(psum > max_psum)
                psum = max_psum;
              else if(psum < min_psum)
                psum = min_psum;
            }
          }
          if(psum > max_out)
            o_Wsum[K*(2*j_j + j_i) + i_k]->write(max_out);
          else if(psum < min_out)
            o_Wsum[K*(2*j_j + j_i) + i_k]->write(min_out);
          else
            o_Wsum[K*(2*j_j + j_i) + i_k]->write(psum);
        }
    //cout << "debug: Synapse/partial_sum_2XY: pos" << endl;
  }
}

void Synapse::data_transfer_C(sc_fifo< sc_int<IN_WIDTH> > **fifo_r, sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w1,
                                                                    sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w2){
  for(int i_c = 0; i_c < C; i_c++){
    sc_int<IN_WIDTH> data_tmp = fifo_r[i_c]->read();
    if(&vector_w1 != &v_null)
      vector_w1[i_c].write(data_tmp);
    if(&vector_w2 != &v_null)
      vector_w2[i_c].write(data_tmp);
  }
}

void Synapse::data_transfer_C(sc_fifo< sc_int<IN_WIDTH> > **fifo_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w,
                                                                    sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w1,
                                                                    sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w2){
  for(int i_c = 0; i_c < C; i_c++){
    sc_int<IN_WIDTH> data_tmp = fifo_r[i_c]->read();
    fifo_w[i_c]->write(data_tmp);
    if(&vector_w1 != &v_null)
      vector_w1[i_c].write(data_tmp);
    if(&vector_w2 != &v_null)
      vector_w2[i_c].write(data_tmp);
  }
}

void Synapse::data_transfer_C(sc_fifo< sc_int<IN_WIDTH> > **fifo_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w1,
                                                                    sc_fifo< sc_int<IN_WIDTH> > **fifo_w2,
                                                                    sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w1,
                                                                    sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w2,
                                                                    sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w3,
                                                                    sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w4){
  for(int i_c = 0; i_c < C; i_c++){
    sc_int<IN_WIDTH> data_tmp = fifo_r[i_c]->read();
    fifo_w1[i_c]->write(data_tmp);
    fifo_w2[i_c]->write(data_tmp);
    if(&vector_w1 != &v_null)
      vector_w1[i_c].write(data_tmp);
    if(&vector_w2 != &v_null)
      vector_w2[i_c].write(data_tmp);
    if(&vector_w3 != &v_null)
      vector_w3[i_c].write(data_tmp);
    if(&vector_w4 != &v_null)
      vector_w4[i_c].write(data_tmp);
  }
}

void Synapse::data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w1,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w2){
  for(int i_c = 0; i_c < C; i_c++){
    sc_int<IN_WIDTH> data_tmp = vector_r[i_c + C_sft]->read();
    if(&vector_w1 != &v_null)
      vector_w1[i_c].write(data_tmp);
    if(&vector_w2 != &v_null)
      vector_w2[i_c].write(data_tmp);
  }
}

void Synapse::data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w1,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w2){
  for(int i_c = 0; i_c < C; i_c++){
    sc_int<IN_WIDTH> data_tmp = vector_r[i_c + C_sft]->read();
    fifo_w[i_c]->write(data_tmp);
    if(&vector_w1 != &v_null)
      vector_w1[i_c].write(data_tmp);
    if(&vector_w2 != &v_null)
      vector_w2[i_c].write(data_tmp);
  }
}

void Synapse::data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w1,
                                                                                                              sc_fifo< sc_int<IN_WIDTH> > **fifo_w2,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w1,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w2){
  for(int i_c = 0; i_c < C; i_c++){
    sc_int<IN_WIDTH> data_tmp = vector_r[i_c + C_sft]->read();
    fifo_w1[i_c]->write(data_tmp);
    fifo_w2[i_c]->write(data_tmp);
    if(&vector_w1 != &v_null)
      vector_w1[i_c].write(data_tmp);
    if(&vector_w2 != &v_null)
      vector_w2[i_c].write(data_tmp);
  }
}

void Synapse::data_transfer_C(int C_sft, sc_vector< sc_port< sc_fifo_in_if< sc_int<IN_WIDTH> > > > &vector_r, sc_fifo< sc_int<IN_WIDTH> > **fifo_w1,
                                                                                                              sc_fifo< sc_int<IN_WIDTH> > **fifo_w2,
                                                                                                              sc_fifo< sc_int<IN_WIDTH> > **fifo_w3,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w1,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w2,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w3,
                                                                                                              sc_vector< sc_fifo< sc_int<IN_WIDTH> > > &vector_w4){
  for(int i_c = 0; i_c < C; i_c++){
    sc_int<IN_WIDTH> data_tmp = vector_r[i_c + C_sft]->read();
    fifo_w1[i_c]->write(data_tmp);
    fifo_w2[i_c]->write(data_tmp);
    fifo_w3[i_c]->write(data_tmp);
    if(&vector_w1 != &v_null)
      vector_w1[i_c].write(data_tmp);
    if(&vector_w2 != &v_null)
      vector_w2[i_c].write(data_tmp);
    if(&vector_w3 != &v_null)
      vector_w3[i_c].write(data_tmp);
    if(&vector_w4 != &v_null)
      vector_w4[i_c].write(data_tmp);
  }
}
