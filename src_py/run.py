import os
import sys
import logging
import numpy as np
import argparse

import util
from init import Init
import CIFAR10_to_txt
import fp_graph
import tr_graph
import inf_graph

parser = argparse.ArgumentParser('SAsim')
parser.add_argument('--gpu', type=int, default=2, help='')#1
parser.add_argument('--path_dataset', type=str, default='cifar10', help='')
parser.add_argument('--path_dataset_txt', type=str, default='txt_CIFAR10', help='')
parser.add_argument('--batch_size_txt', type=int, default=10, help='batch size in txt file, -1 for whole test set')#-1
parser.add_argument('--path_output_file', type=str, default='my_file', help='')
parser.add_argument('--path_saved_net', type=str, default='my_net', help='')
parser.add_argument('--path_data_txt', type=str, default='txt_data', help='')
parser.add_argument('--path_sc_config', type=str, default='src_sc/dir_config', help='')
parser.add_argument('--DNN_model', type=str, default='DebugNet', help='')#'TheAllConvNet'
parser.add_argument('--layer_cfg', type=str, default='cp4', help='')
parser.add_argument('--RRAM_array', type=bool, default=True, help='')#False
parser.add_argument('--w_qn_bits', type=int, default=7, help='weight quantization bits') # 8 -> 3
parser.add_argument('--b_qn_bits', type=int, default=-1, help='bias quantization bits, -1 for w_qn_bits + 8')
parser.add_argument('--a_qn_bits', type=int, default=7, help='activation quantization bits') # 8 -> 3
parser.add_argument('--export_txt', type=bool, default=True, help='')#True
parser.add_argument('--learning_rate_max', type=float, default=0.1, help='')
parser.add_argument('--half_decay_epoch', type=float, default=1, help='')
parser.add_argument('--epsilon_bn', type=float, default=0.001, help='')
parser.add_argument('--momentum', type=float, default=0.9, help='')
parser.add_argument('--lr_decay_rate', type=float, default=0.1, help='')
parser.add_argument('--n_batch', type=int, default=50, help='batch size for pre-training/training step')
parser.add_argument('--n_batch_inf', type=int, default=100, help='batch size for inference step')
parser.add_argument('--n_epoch_fp', type=int, default=30, help='training epochs for pre-training step')#300
parser.add_argument('--n_epoch_tr', type=int, default=10, help='training epochs for training step')#10
#parser.add_argument('--', type=, default=, help='')
args = parser.parse_args()

if args.b_qn_bits == -1:
	args.b_qn_bits = args.w_qn_bits + 8

args.init = Init(args)

log_format = '%(asctime)s %(message)s'
logging.basicConfig(stream=sys.stdout, level=logging.INFO,
    format=log_format, datefmt='%m/%d %I:%M:%S %p')
fh = logging.FileHandler('log.txt')
fh.setFormatter(logging.Formatter(log_format))
logging.getLogger().addHandler(fh)

func = os.system
#func = logging.info # debug

if __name__ == '__main__':
	CIFAR10_to_txt.main(args)#done
	fp_graph.main(args)#done
	tr_graph.main(args)#done
	inf_graph.main(args)#done

	str_imp = 'RRAM' if args.RRAM_array else 'digPE'
	
	file_csv = 'res_{}_{}_{}_w{}a{}.csv'.format(args.DNN_model, args.layer_cfg, str_imp, args.w_qn_bits, args.a_qn_bits)
	
	util.file_ctor_CFG(args, file_csv)
	func('cat {0}/config_seg1.h {0}/config_seg2.h > src_sc/config.h'.format(args.path_sc_config))
	func('make clean')
	if not os.path.isdir('build/'):
		os.mkdir('build/')
	func('make exe')
	if not os.path.isdir('build_exe/'):
		os.mkdir('build_exe/')
	func('mv build/main.exe build_exe/main_{}_{}_{}_w{}a{}.exe'.format(args.DNN_model, args.layer_cfg, str_imp, args.w_qn_bits, args.a_qn_bits))
	func('./build_exe/main_{}_{}_{}_w{}a{}.exe'.format(args.DNN_model, args.layer_cfg, str_imp, args.w_qn_bits, args.a_qn_bits))
	
	util.label_accuracy(args, file_csv)
	#func('diff {} {}'.format(file_out, file_out_ref))
	
