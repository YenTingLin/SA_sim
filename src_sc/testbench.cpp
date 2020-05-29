#include "testbench.h"

Testbench::Testbench(const sc_module_name &mn, int mul_C, int num_layers, int c, int k)
  : sc_module(mn), L(num_layers), C(c), K(k), mul_C(mul_C),
  o_fmap("o_fmap", mul_C*c), i_Wsum("i_Wsum", k){
  cout << "debug: Testbench/Testbench: pre" << endl;
  read_filter();
  switch(mul_C){
    case 2:
      SC_THREAD(read_ifmap_2); break;
    case 4:
      SC_THREAD(read_ifmap_4); break;
    default:
      SC_THREAD(read_ifmap); break;
  }
  SC_THREAD(dump_ofmap);
  cout << "debug: Testbench/Testbench: pos" << endl;
}

Testbench::~Testbench(){
  for(int i_l = 0; i_l < L_real; i_l++){
    if(shape_weights[i_l][0] == 0)
      continue;
    for(int i_i = 0; i_i < shape_weights[i_l][0]; i_i++){
      for(int i_j = 0; i_j < shape_weights[i_l][1]; i_j++){
        for(int i_c = 0; i_c < shape_weights[i_l][2]; i_c++){
          delete [] weights[i_l][i_i][i_j][i_c];
        }
        delete [] weights[i_l][i_i][i_j];
      }
      delete [] weights[i_l][i_i];
    }
    delete [] weights[i_l];
    delete [] shape_weights[i_l];
    delete [] bias[i_l];
    delete [] scale[i_l];
  }
  delete [] weights;
  delete [] shape_weights;
  delete [] bias;
  delete [] shape_bias;
  delete [] scale;
}

void Testbench::read_filter(){
  cout << "debug: Testbench/read_filter: pre" << endl;
  ifstream fin(FILTER_TXT);
  int I_, J_, C_, K_;
  int M_m_mul, M_m_sft, M_b_mul, M_b_sft;
  int data;
  string st_tmp;
  weights = new int ****[L];
  shape_weights = new int *[L];
  bias = new int *[L];
  shape_bias = new int [L];
  scale = new int *[L];
  for(int i_l = 0; i_l < L; i_l++){
    fin >> st_tmp;
    //cout << "debug: Testbench/read_filter: " << st_tmp << endl;
    if(st_tmp == "FINISH"){
      L_real = i_l;
      break;
    }
    fin >> I_;
    if(I_ == 0){
      shape_weights[i_l] = new int [1];
      shape_weights[i_l][0] = 0;
      shape_bias[i_l] = 0;
      continue;
    }
    fin >> J_ >> C_ >> K_;
    cout << "debug: Testbench/read_filter: I_J_C_K_= " << I_ << " " << J_ << " "  << C_ << " "  << K_ << endl;
    int *shape = new int [4];
    shape[0] = I_;
    shape[1] = J_;
    shape[2] = C_;
    shape[3] = K_;
    shape_weights[i_l] = shape;
    fin >> st_tmp;
    cout << "debug: Testbench/read_filter: " << st_tmp << endl;
    weights[i_l] = new int ***[I_];
    for(int i_i = 0; i_i < I_; i_i++){
      weights[i_l][i_i] = new int **[J_];
      for(int i_j = 0; i_j < J_; i_j++){
        weights[i_l][i_i][i_j] = new int *[C_];
        for(int i_c = 0; i_c < C_; i_c++){
          weights[i_l][i_i][i_j][i_c] = new int[K_];
          for(int i_k = 0; i_k < K_; i_k++){
            fin >> data;
            weights[i_l][i_i][i_j][i_c][i_k] = data;
            //cout << "debug: Testbench/read_filter: weights:" << data << endl;
            //cout << "debug: Testbench/read_filter: weights:" << weights[i_l][i_i][i_j][i_c][i_k] << endl;
          }
        }
      }
    }
    fin >> st_tmp;
    fin >> M_m_mul >> M_m_sft;
    fin >> st_tmp;
    fin >> K_;
    //cout << "debug: Testbench/read_filter: K_= " << K_ << endl;
    shape_bias[i_l] = K_;
    fin >> st_tmp;
    //cout << "debug: Testbench/read_filter: " << st_tmp << endl;
    bias[i_l] = new int [K_];
    for(int i_k = 0; i_k < K_; i_k++){
      fin >> data;
      bias[i_l][i_k] = data;
      //cout << "debug: Testbench/read_filter: bias:" << data << endl;
      //cout << "debug: Testbench/read_filter: bias:" << bias[i_l][i_k] << endl;
    }
    fin >> st_tmp;
    fin >> M_b_mul >> M_b_sft;
    scale[i_l] = new int[4]; // {M_m_mul, M_m_sft, M_b_mul, M_b_sft}
    scale[i_l][0] = M_m_mul;
    scale[i_l][1] = M_m_sft;
    scale[i_l][2] = M_b_mul;
    scale[i_l][3] = M_b_sft;
  }
  fin.close();
  cout << "debug: Testbench/read_filter: pos" << endl;
}

void Testbench::dump_cycle_count(Layer_property *properties, int num_layers){
  ofstream fout(CYCLE_CNT_CSV);
  long long int cycle_cal;
  fout << "layer name,layer type,I,J,C,K,St,X0,Y0,X,Y,arch type,PE number,RRAM array number,"
       << "skip_L,skip_W,"
       << "total cycle,wait cycle,valid cal cycle,invalid cal cycle,U_cal,U_total" << endl;
  for(int i = 0; i < num_layers; i++){
    cycle_cal = properties[i].cycle_cal_v + properties[i].cycle_cal_inv;
    fout << properties[i].name << "," << properties[i].type << "," << properties[i].I << "," << properties[i].J << "," << properties[i].C << ",";
    fout << properties[i].K << "," << properties[i].St << "," << properties[i].X0 << "," << properties[i].Y0 << "," << properties[i].X << ",";
    fout << properties[i].Y << "," << properties[i].ptype << "," << properties[i].P_total << "," <<  properties[i].num_RRAM << ",";
    fout << properties[i].skip_L << "," << properties[i].skip_W << "," << cycle_total << ",";
    fout << cycle_total - cycle_cal << "," << properties[i].cycle_cal_v << "," << properties[i].cycle_cal_inv << ",";
    fout << properties[i].cycle_cal_v / (double)cycle_cal << "," << properties[i].cycle_cal_v / (double)cycle_total << endl;
  }
  fout.close();
  delete [] properties;
}

void Testbench::read_ifmap(){
  cout << "debug: Testbench/read_ifmap: pre" << endl;
  ifstream fin(IFMAP_TXT);
  int X0_, Y0_, C_;
  int data;
  string st_tmp;
  //o_rst->write(true);
  t_begin = sc_time_stamp();
  fin >> st_tmp;
  fin >> B >> X0_ >> Y0_ >> C_;
  fin >> st_tmp;
  for(int i_b = 0; i_b < B; i_b++){
    for(int i_x = 0; i_x < X0_; i_x++){
      cout << "debug: Testbench/read_ifmap: (" << i_b << ", " << i_x << ", : )" << endl;
      for(int i_y = 0; i_y < Y0_; i_y++){
        //cout << "debug: Testbench/read_ifmap: (" << i_b << ", " << i_x << ", " << i_y << ")" << endl;
        for(int i_c = 0; i_c < C; i_c++){
          fin >> data;
          o_fmap[i_c]->write(data - 128); // uint8 -> int8
        }
        //o_rst->write(false);
      }
    }
  }
  fin.close();
  cout << "debug: Testbench/read_ifmap: pos" << endl;
}

void Testbench::read_ifmap_2(){
  cout << "debug: Testbench/read_ifmap_2: pre" << endl;
  ifstream fin(IFMAP_TXT);
  int X0_, Y0_, C_;
  int data;
  string st_tmp;
  t_begin = sc_time_stamp();
  fin >> st_tmp;
  fin >> B >> X0_ >> Y0_ >> C_;
  fin >> st_tmp;
  for(int i_b = 0; i_b < B; i_b++){
    for(int i_x = 0; i_x < Y0_; i_x++){
      cout << "debug: Testbench/read_ifmap_2: (" << i_b << ", " << i_x << ", : )" << endl;
      for(int i_y = 0; i_y < Y0_; i_y+=2){
        //cout << "debug: Testbench/read_ifmap_2: (" << i_b << ", " << i_x << ", " << i_y << "-" << i_y + 1 << ")" << endl;
        for(int j_y = 0; j_y < 2; j_y++){
          if(i_y + j_y < Y0_)
            for(int i_c = 0; i_c < C; i_c++){
              fin >> data;
              o_fmap[C*j_y + i_c]->write(data - 128); // uint8 -> int8
            }
          else
            for(int i_c = 0; i_c < C; i_c++)
              o_fmap[C*j_y + i_c]->write(0);
        }
      }
    }
  }
  fin.close();
  cout << "debug: Testbench/read_ifmap_2: pos" << endl;
}

void Testbench::read_ifmap_4(){
  cout << "debug: Testbench/read_ifmap_4: pre" << endl;
  ifstream fin(IFMAP_TXT);
  int X0_, Y0_, C_;
  int data;
  string st_tmp;
  t_begin = sc_time_stamp();
  fin >> st_tmp;
  fin >> B >> X0_ >> Y0_ >> C_;
  fin >> st_tmp;
  for(int i_b = 0; i_b < B; i_b++){
    for(int i_x = 0; i_x < Y0_; i_x+=2){
      cout << "debug: Testbench/read_ifmap_4: (" << i_b << ", " << i_x << "-" << i_x + 1 << ", : )" << endl;
      for(int i_y = 0; i_y < Y0_; i_y+=2){
        //cout << "debug: Testbench/read_ifmap_4: (" << i_b << ", " << i_x << "-" << i_x + 1 << ", " << i_y << "-" << i_y + 1 << ")" << endl;
        for(int j_x = 0; j_x < 2; j_x++){
          for(int j_y = 0; j_y < 2; j_y++){
            if(i_x + j_x < X0_ && i_y + j_y < Y0_)
              for(int i_c = 0; i_c < C; i_c++){
                fin >> data;
                o_fmap[C*(2*j_x + j_y) + i_c]->write(data - 128); // uint8 -> int8
              }
            else
              for(int i_c = 0; i_c < C; i_c++)
                o_fmap[C*(2*j_x + j_y) + i_c]->write(0);
          }
        }
      }
    }
  }
  fin.close();
  cout << "debug: Testbench/read_ifmap_4: pos" << endl;
}

void Testbench::dump_ofmap(){
  cout << "debug: Testbench/dump_ofmap: pre" << endl;
  ofstream fout(OFMAP_TXT);
  int data;
  fout << "ofmap:size" << endl;
  fout << B << " " << K << " " << endl;
  fout << "ofmap:value" << endl;
  for(int i_b = 0; i_b < B; i_b++){
    cout << "debug: Testbench/dump_ofmap: i_Wsum[ : ]" << endl;
    for(int i_k = 0; i_k < K; i_k++){
      data = i_Wsum[i_k]->read();
      //cout << "debug: Testbench/dump_ofmap: i_Wsum[" << i_k << "]=" << data << endl;
      fout << data << endl;
    }
  }
  cout << "debug: Testbench/dump_ofmap: pos" << endl;
  fout.close();
  sc_stop();
  cycle_total = (sc_time_stamp() - t_begin).value() / 1000;
  cout << "Timing: " << cycle_total << endl;
}

ostream &operator<<(ostream &sout, Parallel_type ptype){
  switch(ptype){
    case pt_2YS: case pt_2Ym:
      sout << "2YS"; break;
    case pt_2XYS: case pt_2XYm:
      sout << "2XYS"; break;
    case pt_2Y:
      sout << "2Y"; break;
    case pt_2XY:
      sout << "2XY"; break;
    default:
      sout << "none";
  }
  return sout;
}
