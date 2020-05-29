import tensorflow as tf
import numpy as np
import logging

import layer_def

# ----- Define Neural Network -----#
def def_TheAllConvNet_cp1(l_p_keep, tuple_args):
	init = tuple_args[3].init
	k1 = 96
	k2 = 192
	p_keep_in = l_p_keep[0]
	p_keep_pool = l_p_keep[1]

	layer_def.dropout(p_keep_in, tuple_args, name='drop0')
	layer_def.layer_conv(3, k1, 1, 'SAME', 'p1', tuple_args, name='layerC0')
	layer_def.layer_conv(3, k1, 1, 'SAME', 'p1', tuple_args, name='layerC1')
	layer_def.layer_conv(3, k1, 2, 'SAME', 'p1', tuple_args, name='layerP2')
	layer_def.dropout(p_keep_pool, tuple_args, name='drop1')
	layer_def.layer_conv(3, k2, 1, 'SAME', 'p1', tuple_args, name='layerC3')
	layer_def.layer_conv(3, k2, 1, 'SAME', 'p1', tuple_args, name='layerC4')
	layer_def.layer_conv(3, k2, 2, 'SAME', 'p1', tuple_args, name='layerP5')
	layer_def.dropout(p_keep_pool, tuple_args, name='drop2')
	layer_def.layer_conv(3, k2, 1, 'SAME', 'p1', tuple_args, name='layerC6')
	layer_def.layer_conv(1, k2, 1, 'SAME', 'p1', tuple_args, name='layerC7')
	layer_def.layer_conv(1, init.dataset.K, 1, 'SAME', 'p1', tuple_args, name='layerC8')
	layer_def.layer_global_avg(tuple_args, name='layer9')

def def_DebugNet_cp1(l_p_keep, tuple_args):
	init = tuple_args[3].init
	k1 = 24
	k2 = 48
	p_keep_in = l_p_keep[0]
	p_keep_pool = l_p_keep[1]

	layer_def.dropout(p_keep_in, tuple_args, name='drop0')
	layer_def.layer_conv(3, k1, 1, 'SAME', 'p1', tuple_args, name='layerC0')
	layer_def.layer_conv(3, k1, 2, 'SAME', 'p1', tuple_args, name='layerP1')
	layer_def.dropout(p_keep_pool, tuple_args, name='drop1')
	layer_def.layer_conv(3, k2, 1, 'SAME', 'p1', tuple_args, name='layerC2')
	layer_def.layer_conv(3, k2, 2, 'SAME', 'p1', tuple_args, name='layerP3')
	layer_def.dropout(p_keep_pool, tuple_args, name='drop2')
	layer_def.layer_conv(3, k2, 1, 'SAME', 'p1', tuple_args, name='layerC4')
	layer_def.layer_conv(1, init.dataset.K, 1, 'SAME', 'p1', tuple_args, name='layerC5')
	layer_def.layer_global_avg(tuple_args, name='layer6')

def def_DebugNet_cp4(l_p_keep, tuple_args):
	init = tuple_args[3].init
	k1 = 24
	k2 = 48
	p_keep_in = l_p_keep[0]
	p_keep_pool = l_p_keep[1]

	layer_def.dropout(p_keep_in, tuple_args, name='drop0')
	layer_def.layer_conv(3, k1, 1, 'SAME', 'p4', tuple_args, name='layerC0')
	layer_def.layer_conv(3, k1, 2, 'SAME', 'p4', tuple_args, name='layerP1')
	layer_def.dropout(p_keep_pool, tuple_args, name='drop1')
	layer_def.layer_conv(3, k2, 1, 'SAME', 'p1', tuple_args, name='layerC2')
	layer_def.layer_conv(3, k2, 2, 'SAME', 'p4', tuple_args, name='layerP3')
	layer_def.dropout(p_keep_pool, tuple_args, name='drop2')
	layer_def.layer_conv(3, k2, 1, 'SAME', 'p1', tuple_args, name='layerC4')
	layer_def.layer_conv(1, init.dataset.K, 1, 'SAME', 'p1', tuple_args, name='layerC5')
	layer_def.layer_global_avg(tuple_args, name='layer6')

def def_Net(l_p_keep, tuple_args):
	if tuple_args[3].DNN_model == 'TheAllConvNet':
		if tuple_args[3].layer_cfg == 'cp1':
			def_TheAllConvNet_cp1(l_p_keep, tuple_args)
	elif tuple_args[3].DNN_model == 'DebugNet':
		if tuple_args[3].layer_cfg == 'cp1':
			def_DebugNet_cp1(l_p_keep, tuple_args)
		elif tuple_args[3].layer_cfg == 'cp4':
			def_DebugNet_cp4(l_p_keep, tuple_args)
	else:
		logging.info('ERROR: fp_grapg: invalid DNN type!')
		exit(9487)
