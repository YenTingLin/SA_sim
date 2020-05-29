#define OUT_WIDTH 32//32, weight + act + b_mrg
#define SCALE_WIDTH 16
#define PSUM_WIDTH 32//32, weight + act + b_mrg

#include "systemc.h"
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <climits>
#include <algorithm>
#include <string>
#include <sstream>

using namespace std;

//const int bit_mrg[] = {2, 7, 7, 7, 8, 8, 8, 8, 8, 6};
//const int bit_mrg[] = {5, 10, 10, 10, 11, 11, 11, 8, 8, 6};
//const int bit_mrg[] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

enum Parallel_type{none, pt_2XS, pt_2YS, pt_2XYS, pt_2X, pt_2Y, pt_2XY, pt_2Xm, pt_2Ym, pt_2XYm};
enum Skip_type{st_none, st_ext, st_par, st_deg};

struct Layer_property{
  string name;
  string type;
  int I,J,C,K,St,X0,Y0,X,Y;
  long long int P_total, num_RRAM; // PEs, RRAM arrays count
  int skip_L, skip_W; // fifo length, width of skip path
  long long int cycle_cal_v, cycle_cal_inv; // cycle count
  Parallel_type ptype;
};

struct Array{
  int *data;
  int len;

  Array(int len_in):
    data(new int[len_in]), len(len_in){}

  Array():
    data(new int[0]), len(0){}

  ~Array(){
    //delete [] data;
  }

  void fill(Array a_in){
    if(len > a_in.len){
      for(int i = 0; i < a_in.len; i++)
        data[i] = a_in.data[i];
    }
    else{
      for(int i = 0; i < len; i++)
        data[i] = a_in.data[i];
    }
  }

  int operator[](int index){
    return data[index];
  }

  void print(){
    cout << "[ ";
    if(len > 0){
      for(int i = 0; i < len; i++)
        cout << data[i] << ", ";
      cout << "\b\b ]" << endl;
    }
    else
      cout << " ]" << endl;
  }

  int sum(){
    int sum = 0;
    for(int i = 0; i < len; ++i)
      sum += data[i];
    return sum;
  }

  int sum_1(){
    int sum = 0;
    for(int i = 0; i < len-1; ++i)
      sum += data[i];
    return sum;
  }
};

#endif
