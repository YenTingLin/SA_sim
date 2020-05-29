#ifndef _CONFIG_H_
#define _CONFIG_H_

#define IFMAP_TXT "txt_CIFAR10/image.txt"

#define FILTER_TXT "txt_data/DebugNet/test_file_w7a7/filter.txt"
#define OFMAP_TXT "txt_data/DebugNet/test_file_w7a7/out.txt"
#define CYCLE_CNT_CSV "res_DebugNet_cp4_RRAM_w7a7.csv"

#define CLK_CYCLE 49 // cycle time = 10 ns
#define PADDING_IN_CYCLE 1
#define PADDING_OUT_CYCLE 1
#define ACT_CYCLE 1
#define POOL_CYCLE 1

#define IN_WIDTH 8
#define WEIGHT_WIDTH 7
#define BIAS_WIDTH 15
#define mul_C_in 4
