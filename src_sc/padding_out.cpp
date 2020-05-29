#include "padding_out.h"

Padding_out::Padding_out(const sc_module_name &mn, int k, int i, int j, int x0, int y0, int stride, int bit_mrg_, Parallel_type ptype)
  : sc_module(mn), I(i), J(j), K(k), X0(x0), Y0(y0), St(stride),
  num_max((1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1)) - 1), // 2^31 - 1
  num_min(-(1LL << (IN_WIDTH + WEIGHT_WIDTH + bit_mrg_ - 1))), // - 2^31
  cycle_cal_v(0), cycle_cal_inv(0),
  i_data("i_data"), o_data("o_data"){
  //cout << "debug: Padding_out/Padding_out: pre" << endl;
  switch(ptype){
    case pt_2YS: case pt_2Ym:
      i_data.init(k);
      o_data.init(k);
      SC_THREAD(data_transfer_2YS);
      break;
    case pt_2XYS: case pt_2XYm:
      i_data.init(k);
      o_data.init(k);
      SC_THREAD(data_transfer_2XYS);
      break;
    case pt_2Y:
      i_data.init(2*k);
      o_data.init(2*k);
      SC_THREAD(data_transfer_2Y);
      //SC_THREAD(data_transfer_D);//debug
      break;
    case pt_2XY:
      i_data.init(4*k);
      o_data.init(4*k);
      SC_THREAD(data_transfer_2XY);
      break;
    default:
      i_data.init(k);
      o_data.init(k);
      SC_THREAD(data_transfer);
  }
  //cout << "debug: Padding_out/Padding_out: pos" << endl;
}

void Padding_out::map_scale(int *scale){
  M_m_mul = scale[0];
  M_m_sft = scale[1];
}

void Padding_out::data_transfer(){
  while(true){
    //cout << "debug: Padding_out/data_transfer: pre" << endl;
    for(int i_x = 0; i_x < X0; i_x++)
      for(int i_y = 0; i_y < Y0; i_y++){
        bool en_valid, en_evict;
        en_valid = i_x >= I-1 && i_y >= J-1;
        if(en_valid)
          en_evict = (i_x - I + 1) % St != (St-1) || (i_y - J + 1) % St != (St-1);
        else
          en_evict = true;
        if(en_evict){
          for(int i_k = 0; i_k < K; i_k++){
            //cout << "debug: Padding_out/data_transfer: 1" << endl;
            i_data[i_k]->read();
            //cout << "debug: Padding_out/data_transfer: 2" << endl;
          }
          cycle_cal_inv++; // invalid cal. cycle
        }
        else{
          for(int i_k = 0; i_k < K; i_k++){
            //cout << "debug: Padding_out/data_transfer: 3" << endl;
            data_tmp = i_data[i_k]->read();
            /**/
            data_tmp *= M_m_mul;
            if(M_m_sft > 0)
              data_tmp = data_tmp << M_m_sft;
            else if(M_m_sft < 0)
              data_tmp = data_tmp >> (-M_m_sft);
            if(data_tmp > num_max)
              data_tmp = num_max;
            else if(data_tmp < num_min)
              data_tmp = num_min;
            /**/
            o_data[i_k]->write(data_tmp);
            //cout << "debug: Padding_out/data_transfer: 4" << endl;
          }
          cycle_cal_v++; // valid cal. cycle
        }
        wait(PADDING_OUT_CYCLE, SC_NS);
      }
    //cout << "debug: Padding_out/data_transfer: pos" << endl;
  }
}

void Padding_out::data_transfer_D(){
  while(true){
    //cout << "debug: Padding_out/data_transfer_D: pre" << endl;
    for(int i = 0; i < 100; i++){
      if(i < 50)
        for(int i_k = 0; i_k < 2*K; i_k++)
          i_data[i_k]->read();
      if(i < 100)
        for(int i_k = 0; i_k < 2*K; i_k++)
          o_data[i_k]->write(1);
      wait(PADDING_OUT_CYCLE, SC_NS);
    }
    //cout << "debug: Padding_out/data_transfer_D: pos" << endl;
  }
}

void Padding_out::data_transfer_2YS(){
  while(true){
    //cout << "debug: Padding_out/data_transfer_2YS: pre" << endl;
    for(int i_x = 0; i_x < X0; i_x++)
      for(int i_y = 0; i_y < Y0; i_y+=2){
        bool en_valid, en_evict;
        en_valid = i_x >= I-1 && i_y >= J-1;
        if(en_valid)
          en_evict = (i_x - I + 1) % St != (St-1);// || (i_y - J + 1) % St != (St-1);
        else
          en_evict = true;
        if(en_evict){
          for(int i_k = 0; i_k < K; i_k++){
            //cout << "debug: Padding_out/data_transfer_2YS: 1" << endl;
            i_data[i_k]->read();
            //cout << "debug: Padding_out/data_transfer_2YS: 2" << endl;
          }
          cycle_cal_inv++; // invalid cal. cycle
        }
        else{
          for(int i_k = 0; i_k < K; i_k++){
            //cout << "debug: Padding_out/data_transfer_2YS: 3" << endl;
            data_tmp = i_data[i_k]->read();
            /**/
            data_tmp *= M_m_mul;
            if(M_m_sft > 0)
              data_tmp = data_tmp << M_m_sft;
            else if(M_m_sft < 0)
              data_tmp = data_tmp >> (-M_m_sft);
            if(data_tmp > num_max)
              data_tmp = num_max;
            else if(data_tmp < num_min)
              data_tmp = num_min;
            /**/
            o_data[i_k]->write(data_tmp);
            //cout << "debug: Padding_out/data_transfer_2YS: 4" << endl;
          }
          cycle_cal_v++; // valid cal. cycle
        }
        wait(PADDING_OUT_CYCLE, SC_NS);
      }
    //cout << "debug: Padding_out/data_transfer_2YS: pos" << endl;
  }
}

void Padding_out::data_transfer_2XYS(){
  while(true){
    //cout << "debug: Padding_out/data_transfer_2XYS: pre" << endl;
    for(int i_x = 0; i_x < X0; i_x+=2)
      for(int i_y = 0; i_y < Y0; i_y+=2){
        bool en_valid = i_x >= I-1 && i_y >= J-1;
        if(!en_valid){
          for(int i_k = 0; i_k < K; i_k++){
            //cout << "debug: Padding_out/data_transfer_2XYS: 1" << endl;
            i_data[i_k]->read();
            //cout << "debug: Padding_out/data_transfer_2XYS: 2" << endl;
          }
          cycle_cal_inv++; // invalid cal. cycle
        }
        else{
          for(int i_k = 0; i_k < K; i_k++){
            //cout << "debug: Padding_out/data_transfer_2XYS: 3" << endl;
            data_tmp = i_data[i_k]->read();
            /**/
            data_tmp *= M_m_mul;
            if(M_m_sft > 0)
              data_tmp = data_tmp << M_m_sft;
            else if(M_m_sft < 0)
              data_tmp = data_tmp >> (-M_m_sft);
            if(data_tmp > num_max)
              data_tmp = num_max;
            else if(data_tmp < num_min)
              data_tmp = num_min;
            /**/
            o_data[i_k]->write(data_tmp);
            //cout << "debug: Padding_out/data_transfer_2XYS: 4" << endl;
          }
          cycle_cal_v++; // valid cal. cycle
        }
        wait(PADDING_OUT_CYCLE, SC_NS);
      }
    //cout << "debug: Padding_out/data_transfer_2XYS: pos" << endl;
  }
}

void Padding_out::data_transfer_2Y(){
  while(true){
    //cout << "debug: Padding_out/data_transfer_2Y: pre" << endl;
    bool cnt_y = 0;
    for(int i_x = 0; i_x < X0; i_x++)
      for(int i_y = 0; i_y < Y0; i_y+=2){
        bool en_cal_v = false;
        for(int j_y = 0; j_y < 2; j_y++){
          bool en_valid = i_x >= I-1 && i_y + j_y >= J-1;
          if(!en_valid){
            for(int i_k = 0; i_k < K; i_k++){
              //cout << "debug: Padding_out/data_transfer_2Y: 1" << endl;
              i_data[K*j_y + i_k]->read();
              //cout << "debug: Padding_out/data_transfer_2Y: 2" << endl;
            }
          }
          else{
            for(int i_k = 0; i_k < K; i_k++){
              //cout << "debug: Padding_out/data_transfer:_2Y 3" << endl;
              data_tmp = i_data[K*j_y + i_k]->read();
              /**/
              data_tmp *= M_m_mul;
              if(M_m_sft > 0)
                data_tmp = data_tmp << M_m_sft;
              else if(M_m_sft < 0)
                data_tmp = data_tmp >> (-M_m_sft);
              if(data_tmp > num_max)
                data_tmp = num_max;
              else if(data_tmp < num_min)
                data_tmp = num_min;
              /**/
              o_data[K*cnt_y + i_k]->write(data_tmp);
              //cout << "debug: Padding_out/data_transfer_2Y: 4" << endl;
            }
            cnt_y = !cnt_y;
            en_cal_v = true;
          }
        }
        if(en_cal_v)
          cycle_cal_v++; // valid cal. cycle
        else
          cycle_cal_inv++; // invalid cal. cycle
        wait(PADDING_OUT_CYCLE, SC_NS);
      }
    //cout << "debug: Padding_out/data_transfer_2Y: pos" << endl;
  }
}

void Padding_out::data_transfer_2XY(){
  while(true){
    //cout << "debug: Padding_out/data_transfer_2Y: pre" << endl;
    char cnt_xy = 0;
    for(int i_x = 0; i_x < X0; i_x+=2)
      for(int i_y = 0; i_y < Y0; i_y+=2){
        bool en_cal_v = false;
        for(int j_x = 0; j_x < 2; j_x++)
          for(int j_y = 0; j_y < 2; j_y++){
            bool en_valid = i_x + j_x >= I-1 && i_y + j_y >= J-1;
            if(!en_valid){
              for(int i_k = 0; i_k < K; i_k++){
                //cout << "debug: Padding_out/data_transfer_2Y: 1" << endl;
                i_data[K*(2*j_x + j_y) + i_k]->read();
                //cout << "debug: Padding_out/data_transfer_2Y: 2" << endl;
              }
            }
            else{
              for(int i_k = 0; i_k < K; i_k++){
                //cout << "debug: Padding_out/data_transfer:_2Y 3" << endl;
                data_tmp = i_data[K*(2*j_x + j_y) + i_k]->read();
                /**/
                data_tmp *= M_m_mul;
                if(M_m_sft > 0)
                  data_tmp = data_tmp << M_m_sft;
                else if(M_m_sft < 0)
                  data_tmp = data_tmp >> (-M_m_sft);
                if(data_tmp > num_max)
                  data_tmp = num_max;
                else if(data_tmp < num_min)
                  data_tmp = num_min;
                /**/
                o_data[K*cnt_xy + i_k]->write(data_tmp);
                //cout << "debug: Padding_out/data_transfer_2Y: 4" << endl;
              }
              if(cnt_xy < 3)
                cnt_xy++;
              else
                cnt_xy = 0;
              en_cal_v = true;
            }
          }
        if(en_cal_v)
          cycle_cal_v++; // valid cal. cycle
        else
          cycle_cal_inv++; // invalid cal. cycle
        wait(PADDING_OUT_CYCLE, SC_NS);
      }
    //cout << "debug: Padding_out/data_transfer_2Y: pos" << endl;
  }
}
