#include "padding_in.h"

Padding_in::Padding_in(const sc_module_name &mn, int c, int i, int j, int x0, int y0, bool en_zero_padding_in, Parallel_type ptype)
  : sc_module(mn), I(i), J(j), C(c), X0(x0), Y0(y0), en_zero_padding(en_zero_padding_in),
  i_data("i_data"), o_data("o_data"){
  //cout << "debug: Padding_in/Padding_in: pre" << endl;
  switch(ptype){
    case pt_2YS:
      i_data.init(c);
      o_data.init(2*c);
      SC_THREAD(data_transfer_2YS);
      break;
    case pt_2XYS:
      i_data.init(c);
      o_data.init(4*c);
      SC_THREAD(data_transfer_2XYS);
      break;
    case pt_2Y: case pt_2Ym:
      i_data.init(2*c);
      o_data.init(2*c);
      SC_THREAD(data_transfer_2Y);
      //SC_THREAD(data_transfer_D);//debug
      break;
    case pt_2XY: case pt_2XYm:
      i_data.init(4*c);
      o_data.init(4*c);
      SC_THREAD(data_transfer_2XY);
      break;
    default:
      i_data.init(c);
      o_data.init(c);
      SC_THREAD(data_transfer);
  }
  //cout << "debug: Padding_in/Padding_in: pos" << endl;
}

void Padding_in::data_transfer(){
  while(true){
    //cout << "debug: Padding_in/data_transfer: pre" << endl;
    if(en_zero_padding){
      for(int i_x = 0; i_x < X0+I-1; i_x++)
        for(int i_y = 0; i_y < Y0+J-1; i_y++){
          if(i_x < I/2 || i_x >= X0 + I/2 || i_y < J/2 || i_y >= Y0 + J/2){
            for(int i_c = 0; i_c < C; i_c++){
              //cout << "debug: Padding_in/data_transfer: 1" << endl;
              o_data[i_c]->write(0); // zero padding
              //cout << "debug: Padding_in/data_transfer: 2" << endl;
            }
          }
          else{
            for(int i_c = 0; i_c < C; i_c++){
              //cout << "debug: Padding_in/data_transfer: 3" << endl;
              o_data[i_c]->write(i_data[i_c]->read()); // data transfer
              //cout << "debug: Padding_in/data_transfer: 4" << endl;
            }
          }
          wait(PADDING_IN_CYCLE, SC_NS);
        }
    }
    else{
      for(int i_x = 0; i_x < X0; i_x++)
        for(int i_y = 0; i_y < Y0; i_y++){
          for(int i_c = 0; i_c < C; i_c++)
            o_data[i_c]->write(i_data[i_c]->read());
          wait(PADDING_IN_CYCLE, SC_NS);
        }
    }
    //cout << "debug: Padding_in/data_transfer: pos" << endl;
  }
}

void Padding_in::data_transfer_D(){
  while(true){
    //cout << "debug: Padding_in/data_transfer_D: pre" << endl;
    for(int i = 0; i < 100; i++){
      if(i < 100)
        for(int i_c = 0; i_c < 2*C; i_c++)
          i_data[i_c]->read();
      if(i < 100)
        for(int i_c = 0; i_c < 2*C; i_c++)
          o_data[i_c]->write(0);
      wait(PADDING_IN_CYCLE, SC_NS);
    }
    //cout << "debug: Padding_in/data_transfer_D: pos" << endl;
  }
}

void Padding_in::data_transfer_2YS(){
  while(true){
    //cout << "debug: Padding_in/data_transfer_2YS: pre" << endl;
    if(en_zero_padding){
      for(int i_x = 0; i_x < X0+I-1; i_x++)
        for(int i_y = 0; i_y < Y0+J-1; i_y+=2)
          for(int j_y = 0; j_y < 2; j_y++){
            if(i_x < I/2 || i_x >= X0 + I/2 || i_y + j_y < J/2 || i_y + j_y >= Y0 + J/2){
              for(int i_c = 0; i_c < C; i_c++){
                //cout << "debug: Padding_in/data_transfer_2YS: 1" << endl;
                o_data[C*j_y + i_c]->write(0); // zero padding
                //cout << "debug: Padding_in/data_transfer_2YS: 2" << endl;
              }
            }
            else{
              for(int i_c = 0; i_c < C; i_c++){
                //cout << "debug: Padding_in/data_transfer_2YS: 3" << endl;
                o_data[C*j_y + i_c]->write(i_data[i_c]->read()); // data transfer
                //cout << "debug: Padding_in/data_transfer_2YS: 4" << endl;
              }
            }
            wait(PADDING_IN_CYCLE, SC_NS);
          }
    }
    else{
      for(int i_x = 0; i_x < X0; i_x++)
        for(int i_y = 0; i_y < Y0; i_y+=2)
          for(int j_y = 0; j_y < 2; j_y++){
            for(int i_c = 0; i_c < C; i_c++)
              o_data[C*j_y + i_c]->write(i_data[i_c]->read());
            wait(PADDING_IN_CYCLE, SC_NS);
          }
    }
    //cout << "debug: Padding_in/data_transfer_2YS: pos" << endl;
  }
}

void Padding_in::data_transfer_2XYS(){
  while(true){
    //cout << "debug: Padding_in/data_transfer_2XYS: pre" << endl;
    if(en_zero_padding){
      for(int i_x = 0; i_x < X0+I-1; i_x+=2)
        for(int i_y = 0; i_y < Y0+J-1; i_y+=2)
          for(int j_x = 0; j_x < 2; j_x++)
            for(int j_y = 0; j_y < 2; j_y++){
              if(i_x + j_x < I/2 || i_x + j_x >= X0 + I/2 || i_y + j_y < J/2 || i_y + j_y >= Y0 + J/2){
                for(int i_c = 0; i_c < C; i_c++){
                  //cout << "debug: Padding_in/data_transfer_2XYS: 1" << endl;
                  o_data[C*(2*j_x + j_y) + i_c]->write(0); // zero padding
                  //cout << "debug: Padding_in/data_transfer_2XYS: 2" << endl;
                }
              }
              else{
                for(int i_c = 0; i_c < C; i_c++){
                  //cout << "debug: Padding_in/data_transfer_2XYS: 3" << endl;
                  o_data[C*(2*j_x + j_y) + i_c]->write(i_data[i_c]->read()); // data transfer
                  //cout << "debug: Padding_in/data_transfer_2XYS: 4" << endl;
                }
              }
              wait(PADDING_IN_CYCLE, SC_NS);
            }
    }
    else{
      for(int i_x = 0; i_x < X0; i_x+=2)
        for(int i_y = 0; i_y < Y0; i_y+=2)
          for(int j_x = 0; j_x < 2; j_x++)
            for(int j_y = 0; j_y < 2; j_y++){
              for(int i_c = 0; i_c < C; i_c++)
                o_data[C*(2*j_x + j_y) + i_c]->write(i_data[i_c]->read());
              wait(PADDING_IN_CYCLE, SC_NS);
            }
    }
    //cout << "debug: Padding_in/data_transfer_2XYS: pos" << endl;
  }
}

void Padding_in::data_transfer_2Y(){
  while(true){
    //cout << "debug: Padding_in/data_transfer_2Y: pre" << endl;
    if(en_zero_padding){
      bool cnt_y = 0;
      for(int i_x = 0; i_x < X0+I-1; i_x++)
        for(int i_y = 0; i_y < Y0+J-1; i_y+=2){
          for(int j_y = 0; j_y < 2; j_y++){
            if(i_x < I/2 || i_x >= X0 + I/2 || i_y + j_y < J/2 || i_y + j_y >= Y0 + J/2){
              for(int i_c = 0; i_c < C; i_c++){
                //cout << "debug: Padding_in/data_transfer_2Y: 1" << endl;
                o_data[C*j_y + i_c]->write(0); // zero padding
                //cout << "debug: Padding_in/data_transfer_2Y: 2" << endl;
              }
            }
            else{
              for(int i_c = 0; i_c < C; i_c++){
                //cout << "debug: Padding_in/data_transfer_2Y: 3" << endl;
                o_data[C*j_y + i_c]->write(i_data[C*cnt_y + i_c]->read()); // data transfer
                //cout << "debug: Padding_in/data_transfer_2Y: 4" << endl;
              }
              cnt_y = !cnt_y;
            }
          }
          wait(PADDING_IN_CYCLE, SC_NS);
        }
    }
    else{
      for(int i_x = 0; i_x < X0; i_x++)
        for(int i_y = 0; i_y < Y0; i_y+=2){
          for(int j_y = 0; j_y < 2; j_y++)
            for(int i_c = 0; i_c < C; i_c++)
              o_data[C*j_y + i_c]->write(i_data[C*j_y + i_c]->read());
          wait(PADDING_IN_CYCLE, SC_NS);
        }
    }
    //cout << "debug: Padding_in/data_transfer_2Y: pos" << endl;
  }
}

void Padding_in::data_transfer_2XY(){
  while(true){
    //cout << "debug: Padding_in/data_transfer_2XY: pre" << endl;
    if(en_zero_padding){
      char cnt_xy = 0;
      for(int i_x = 0; i_x < X0+I-1; i_x+=2)
        for(int i_y = 0; i_y < Y0+J-1; i_y+=2){
          for(int j_x = 0; j_x < 2; j_x++)
            for(int j_y = 0; j_y < 2; j_y++){
              if(i_x + j_x < I/2 || i_x + j_x >= X0 + I/2 || i_y + j_y < J/2 || i_y + j_y >= Y0 + J/2){
                for(int i_c = 0; i_c < C; i_c++){
                  //cout << "debug: Padding_in/data_transfer_2XY: 1" << endl;
                  o_data[C*(2*j_x + j_y) + i_c]->write(0); // zero padding
                  //cout << "debug: Padding_in/data_transfer_2XY: 2" << endl;
                }
              }
              else{
                for(int i_c = 0; i_c < C; i_c++){
                  //cout << "debug: Padding_in/data_transfer_2XY: 3" << endl;
                  o_data[C*(2*j_x + j_y) + i_c]->write(i_data[C*cnt_xy + i_c]->read()); // data transfer
                  //cout << "debug: Padding_in/data_transfer_2XY 4" << endl;
                }
                if(cnt_xy < 3)
                  cnt_xy++;
                else
                  cnt_xy = 0;
              }
            }
          wait(PADDING_IN_CYCLE, SC_NS);
        }
    }
    else{
      for(int i_x = 0; i_x < X0; i_x+=2)
        for(int i_y = 0; i_y < Y0; i_y+=2){
          for(int j_x = 0; j_x < 2; j_x++)
            for(int j_y = 0; j_y < 2; j_y++)
              for(int i_c = 0; i_c < C; i_c++)
                o_data[C*(2*j_x + j_y) + i_c]->write(i_data[C*(2*j_x + j_y) + i_c]->read());
          wait(PADDING_IN_CYCLE, SC_NS);
        }
    }
    //cout << "debug: Padding_in/data_transfer_2XY: pos" << endl;
  }
}
