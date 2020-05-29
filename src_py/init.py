import tensorflow as tf
import numpy as np
import logging

import load_CIFAR10 as ld
'''
# hyper parameters
en_bn = True
en_dump_txt = True
padding = 'SAME'
learning_rate_max, half_decay_epoch, epsilon_bn, momentum, lr_decay_rate = 0.1, 1, 0.001, 0.9, 0.1 # (5,2,) # 0.15, 75
n_batch = 50#50						### batch size for training ###
n_batch_inf = 100#100 				### batch size for inference ###
n_epoch_fp = 300#239.895#180.238#300#50 	### training epochs for floating-point graph ###
n_epoch_tr = 10#10 					### training epochs for training graph ###
#str_l = ['layer1','layer2','pool1','layer3','layer4','pool2','layer5','layer6','layer7']
#str_l = ['layer1']
'''
# image data
class Init():
	def __init__(self, args):
		self.dataset = ld.fetch_data(args)
		self.inf_img, self.inf_lab = self.dataset.test_next_batch(100)
		self.dataset.i_batch = 0
		self.n_cycle_fp = int(self.dataset.train_images_len() * args.n_epoch_fp / args.n_batch)
		self.n_cycle_tr = int(self.dataset.train_images_len() * args.n_epoch_tr / args.n_batch)
		self.half_decay_cycle = int(self.dataset.train_images_len() * args.half_decay_epoch / args.n_batch)
		self.n_batch = args.n_batch
		print('INFO:', self.n_cycle_fp, self.n_cycle_tr, self.half_decay_cycle)
		self.int_abs_max = (1 << args.w_qn_bits-1) - 1 #127
		self.int_abs_max_b = (1 << args.b_qn_bits-1) - 1 #32767
		self.int_abs_max_a = (1 << args.a_qn_bits-1) - 1 #127
		self.dtype_W = tf.int8
		self.dtype_b = tf.int16

	def next_batch(self):
	    return self.dataset.train_next_batch(self.n_batch)

'''
#scale = 1 << b_FL
sft_Ws = [9,6,6,7,6,6,7,5,2]
sft_bs = [13,13,14,14,14,14,14,15,13] # ori
#sft_bs = [16,15,15,16,15,16,17,14,11] # match
sft_as = [9,9,9,9,10,10,9,9,7]
'''
# inference
#n_batch_inf = 104
#str_tp = 'tr3'


