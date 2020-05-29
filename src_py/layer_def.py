import tensorflow as tf
import numpy as np
import logging

import util

#tuple_args = (stage, l_tensor, l_d_var, args, ema, l_ema_op)
def dropout(p_keep, tuple_args, name):
    if tuple_args[0] == 'inference': # stage
        return
    l_tensor = tuple_args[1]
    tensor_in = l_tensor[-1]
    tensor_out = tf.nn.dropout(tensor_in, p_keep, name=name)
    l_tensor[-1] = tensor_out

def layer_global_avg(tuple_args, name):
    l_tensor = tuple_args[1]
    l_d_var = tuple_args[2]
    tensor_in = l_tensor[-1]
    if tuple_args[0] == 'inference': # stage
        tmp_int16 = tf.cast(tensor_in, dtype=tf.int16)
        tmp_f32 = tf.reduce_sum(tmp_int16, [1,2])/(int(tmp_int16.shape[1])*int(tmp_int16.shape[2]))#e
        tensor_out = tf.cast(tmp_f32, tf.int8)
        b_mrg = np.ceil(np.log2(int(tensor_in.shape[1])*int(tensor_in.shape[2])))
        io_shape = (int(tensor_in.shape[3]), int(tensor_in.shape[1]), 1)
        util.file_ctor_CNN('global_avg', args=tuple_args[3], b_mrg=int(b_mrg), io_shape=io_shape, name=name)
    else:
        tensor_out = tf.reduce_mean(tensor_in, [1,2])
    logging.info('{} > in_shape:{} ; out_shape:{}'.format(name, tensor_in.shape, tensor_out.shape))
    l_tensor.append(tensor_out)
    l_d_var.append(None)

def layer_avg_pool(f_size, strides, padding, tuple_args, name):
    if tuple_args[0] == 'inference': # stage
        layer_conv_inf(f_size, None, strides, padding, *tuple_args[1:4], name) # (f_size, f_num, strides, padding, l_tensor, l_d_var, args, name)
    else:
        layer_avg_pool_train(f_size, strides, padding, *tuple_args[1:], name) # (f_size, strides, padding, l_tensor, l_d_var, args, ema, l_ema_op, name)

def layer_conv(f_size, f_num, strides, padding, para, tuple_args, name):
    if tuple_args[0] == 'pretrain': # stage
        layer_conv_ptr(f_size, f_num, strides, padding, *tuple_args[1:], name) # (f_size, f_num, strides, padding, l_tensor, l_d_var, args, ema, l_ema_op, name)
    elif tuple_args[0] == 'retrain': # stage
        layer_conv_rtr(f_size, f_num, strides, padding, *tuple_args[1:], name) # (f_size, f_num, strides, padding, l_tensor, l_d_var, args, ema, l_ema_op, name)
    else:
        layer_conv_inf(f_size, f_num, strides, padding, para, *tuple_args[1:4], name) # (f_size, f_num, strides, padding, para, l_tensor, l_d_var, args, name)

def layer_avg_pool_train(f_size, strides, padding, l_tensor, l_d_var, args, ema, l_ema_op, name):
    tensor_in = l_tensor[-1]
    W_conv = tf.constant(np.full([f_size,f_size,int(tensor_in.shape[3]),1], 1 / f_size**2, dtype=np.float32), name = 'W_conv_' + name)
    tensor_out = tf.nn.depthwise_conv2d(tensor_in, W_conv, strides=[1,strides,strides,1], padding=padding)
    # ----- Exponential Moving Average: activation ----- #
    min_act = tf.math.reduce_min(tensor_out, name = 'min_act_' + name)
    max_act = tf.math.reduce_max(tensor_out, name = 'max_act_' + name)
    l_ema_op.append(ema.apply([min_act, max_act]))
    min_act_ema = ema.average(min_act)
    max_act_ema = ema.average(max_act)
    abs_max_act = tf.math.maximum(tf.math.abs(min_act_ema), tf.math.abs(max_act_ema))
    _dict = {'W': W_conv, 'name': name, 'min_act_ema': min_act_ema, 'max_act_ema': max_act_ema, 'abs_max_act': abs_max_act}
    logging.info('{} > in_shape:{} ; out_shape:{}'.format(name, tensor_in.shape, tensor_out.shape))
    l_tensor.append(tensor_out)
    l_d_var.append(_dict)

def layer_conv_ptr(f_size, f_num, strides, padding, l_tensor, l_d_var, args, ema, l_ema_op, name):
    #if name[-3:] == 'Avg':
    #    layer_avg(l_tensor, f_size, strides, padding, ema, l_ema_op, l_d_var, name)
    #    return
    tensor_in = l_tensor[-1]
    W_conv = tf.Variable(tf.truncated_normal([f_size,f_size,int(tensor_in.shape[3]),f_num], stddev=0.1), dtype=tf.float32, name = 'W_conv_' + name)
    b_conv = tf.Variable(tf.zeros([f_num]), dtype=tf.float32, name = 'b_conv_' + name)
    h_conv = tf.nn.conv2d(tensor_in, W_conv, strides=[1,strides,strides,1], padding=padding) + b_conv
    # ----- Batch Normalize, Folded ----- #
    out_size = [f_num]
    mu, sigma2 = tf.nn.moments(h_conv,axes=[0,1,2],)
    beta = tf.Variable(tf.zeros(out_size), dtype=tf.float32, name = 'beta_bn_' + name)
    gamma = tf.Variable(tf.ones(out_size), name = 'gamma_bn_' + name)
    W_conv_f = tf.nn.batch_normalization(W_conv, 0, sigma2, 0, gamma, args.epsilon_bn)
    b_conv_f = tf.nn.batch_normalization(b_conv, mu, sigma2, beta, gamma, args.epsilon_bn)
    ## ----- Exponential Moving Average ----- ##
    l_ema_op.append(ema.apply([mu, sigma2]))
    mu_ema = ema.average(mu)
    sigma2_ema = ema.average(sigma2)
    sigma_ema = tf.math.sqrt(sigma2_ema)
    # ----- Convolution 2D, Fold ----- #
    h_conv_f = tf.nn.conv2d(tensor_in, W_conv_f, strides=[1,strides,strides,1], padding=padding) + b_conv_f
    tensor_out = tf.nn.relu(h_conv_f)
    # ----- Exponential Moving Average: activation ----- #
    min_act = tf.math.reduce_min(tensor_out, name = 'min_act_' + name)
    max_act = tf.math.reduce_max(tensor_out, name = 'max_act_' + name)
    l_ema_op.append(ema.apply([min_act, max_act]))
    min_act_ema = ema.average(min_act)
    max_act_ema = ema.average(max_act)
    _dict = {'W': W_conv, 'b': b_conv, 'beta': beta, 'gamma': gamma, 'act': tensor_out, 'mu_ema': mu_ema, 'sigma_ema': sigma_ema, 'min_act_ema': min_act_ema, 'max_act_ema': max_act_ema, 'l_i_skip': [], 'name': name}
    logging.info('{} > in_shape:{} ; out_shape:{}'.format(name, tensor_in.shape, tensor_out.shape))
    l_tensor.append(tensor_out)
    l_d_var.append(_dict)

def layer_conv_rtr(f_size, f_num, strides, padding, l_tensor, l_d_var, args, ema, l_ema_op, name):
    #if name[-3:] == 'Avg':
    #    layer_avg(l_tensor, f_size, strides, padding, ema, l_ema_op, l_d_var, name)
    #    return
    tensor_in = l_tensor[-1]
    W_conv = tf.Variable(tf.truncated_normal([f_size,f_size,int(tensor_in.shape[3]),f_num], stddev=0.1), dtype=tf.float32, name = 'W_conv_' + name)
    b_conv = tf.Variable(tf.zeros([f_num]), dtype=tf.float32, name = 'b_conv_' + name)
    h_conv = tf.nn.conv2d(tensor_in, W_conv, strides=[1,strides,strides,1], padding=padding) + b_conv
    tf.summary.histogram('W_conv_' + name, W_conv)
    tf.summary.histogram('b_conv_' + name, b_conv)
    tf.summary.histogram('h_conv_' + name, h_conv)
    # ----- Batch Normalize, Folded ----- #
    out_size = [f_num]
    mu, sigma2 = tf.nn.moments(h_conv,axes=[0,1,2],)
    beta = tf.Variable(tf.zeros(out_size), dtype=tf.float32, name = 'beta_bn_' + name)
    gamma = tf.Variable(tf.ones(out_size), name = 'gamma_bn_' + name)
    W_conv_f = tf.nn.batch_normalization(W_conv, 0, sigma2, 0, gamma, args.epsilon_bn)
    b_conv_f = tf.nn.batch_normalization(b_conv, mu, sigma2, beta, gamma, args.epsilon_bn)
    ## ----- Exponential Moving Average: mu, sigma ----- ##
    l_ema_op.append(ema.apply([mu, sigma2]))
    mu_ema = ema.average(mu)
    sigma2_ema = ema.average(sigma2)
    sigma_ema = tf.math.sqrt(sigma2_ema)
    # ----- Fake Quantization: Weight, bias ----- #
    abs_max_W = util.get_abs_max(W_conv_f)
    abs_max_b = util.get_abs_max(b_conv_f)
    W_conv_q = tf.quantization.fake_quant_with_min_max_vars(
                                                    W_conv_f,
                                                    -1.*abs_max_W, abs_max_W,
                                                    args.w_qn_bits,
                                                    narrow_range = True)
    b_conv_q = tf.quantization.fake_quant_with_min_max_vars(
                                                    b_conv_f,
                                                    -1.*abs_max_b, abs_max_b,
                                                    args.b_qn_bits,
                                                    narrow_range = True)

    # ----- Convolution 2D, Fold ----- #
    h_conv_f = tf.nn.conv2d(tensor_in, W_conv_q, strides=[1,strides,strides,1], padding=padding) + b_conv_q
    #h_conv_f = tf.nn.conv2d(tensor_in, W_conv_f, strides=[1,strides,strides,1], padding=padding) + b_conv_f
    tensor_out = tf.nn.relu(h_conv_f)
    # ----- Exponential Moving Average: activation ----- #
    min_act = tf.math.reduce_min(tensor_out, name = 'min_act_' + name)
    max_act = tf.math.reduce_max(tensor_out, name = 'max_act_' + name)
    l_ema_op.append(ema.apply([min_act, max_act]))
    min_act_ema = ema.average(min_act)
    max_act_ema = ema.average(max_act)
    # ----- Fake Quantization: activation ----- #
    abs_max_act = tf.math.maximum(tf.math.abs(min_act_ema), tf.math.abs(max_act_ema))
    tensor_out_q = tf.quantization.fake_quant_with_min_max_vars(
                                                    tensor_out,
                                                    -1.*abs_max_act, abs_max_act,
                                                    args.a_qn_bits,
                                                    narrow_range = True)
    # ----- Output ----- #
    tf.summary.histogram('out_' + name, tensor_out_q)
    _dict = {'W': W_conv, 'b': b_conv, 'beta': beta, 'gamma': gamma, 'act': tensor_out_q, 'mu_ema': mu_ema, 'sigma_ema': sigma_ema, 'min_act_ema': min_act_ema, 'max_act_ema': max_act_ema, 'abs_max_act': abs_max_act, 'l_i_skip': [], 'name': name}
    logging.info('{} > in_shape:{} ; out_shape:{}'.format(name, tensor_in.shape, tensor_out.shape))
    l_tensor.append(tensor_out_q)
    l_d_var.append(_dict)

def layer_conv_inf(f_size, f_num, strides, padding, para, l_tensor, l_d_var, args, name):
    tensor_in = l_tensor[-1]
    
    if f_num == None:
        W_conv = tf.Variable(tf.ones([f_size,f_size,int(tensor_in.shape[3]),1], dtype=tf.int8), name = 'W_conv_' + name)
        b_conv = tf.Variable(tf.ones([int(tensor_in.shape[3])], dtype=tf.int16), name = 'b_conv_' + name)
        conv2d = tf.nn.depthwise_conv2d
        b_mrg = np.ceil(np.log2(f_size * f_size))
    else:
        W_conv = tf.Variable(tf.ones([f_size,f_size,int(tensor_in.shape[3]),f_num], dtype=tf.int8), name = 'W_conv_' + name)
        b_conv = tf.Variable(tf.ones([f_num], dtype=tf.int16), name = 'b_conv_' + name)
        conv2d = tf.nn.conv2d
        b_mrg = np.ceil(np.log2(f_size * f_size * int(tensor_in.shape[3])))
    
    lim_psum = 2**(args.w_qn_bits + args.a_qn_bits + b_mrg - 1) # 2**31
    lim_act = 2**(args.a_qn_bits - 1)

    M_m_mul = tf.Variable(tf.constant(1, dtype=tf.int64), name = 'M_m_mul_' + name)
    M_m_sft = tf.Variable(tf.constant(1, dtype=tf.int64), name = 'M_m_sft_' + name)
    M_b_mul = tf.Variable(tf.constant(1, dtype=tf.int64), name = 'M_b_mul_' + name)
    M_b_sft = tf.Variable(tf.constant(1, dtype=tf.int64), name = 'M_b_sft_' + name)
    
    W_conv_f = tf.cast(W_conv, dtype=tf.float32)
    tensor_in_f = tf.cast(tensor_in, dtype=tf.float32)
    h_conv_f = conv2d(tensor_in_f, W_conv_f, strides=[1,strides,strides,1], padding=padding)
    h_conv_f = tf.minimum(tf.maximum(h_conv_f, -lim_psum), lim_psum-1) # 2**31

    h_conv = tf.cast(h_conv_f, dtype=tf.int64) # <---
    h_conv = h_conv * M_m_mul
    h_conv = tf.bitwise.left_shift(h_conv, M_m_sft)
    h_conv = tf.bitwise.right_shift(h_conv, -1*M_m_sft)
    h_conv = tf.minimum(tf.maximum(h_conv, -lim_psum), lim_psum-1) # 2**31
    h_conv = tf.cast(h_conv, dtype=tf.int32) # <---

    h_conv = h_conv + tf.cast(b_conv, dtype=tf.int32)
    if f_num != None:
        h_conv = tf.nn.relu(h_conv) # must not relu6()

    h_conv = tf.cast(h_conv, dtype=tf.int64) # <---
    h_conv = h_conv * M_b_mul
    h_conv = tf.bitwise.left_shift(h_conv, M_b_sft)
    h_conv = tf.bitwise.right_shift(h_conv, -1*M_b_sft)
    h_conv = tf.minimum(tf.maximum(h_conv, -lim_act), lim_act-1) # 2**7
    h_conv = tf.cast(h_conv, dtype=tf.int8) # <---

    if len(l_d_var) == 0:
        cfg_o = None
        if para == 'p1':
            args.mul_C = 1
        elif para == 'p2':
            args.mul_C = 2
        elif para == 'p4':
            args.mul_C = 4
    else:
        cfg_o = l_d_var[-1]['para']
    io_shape = (int(tensor_in.shape[3]), int(tensor_in.shape[1]), int(h_conv.shape[1]))
    util.file_ctor_CNN('conv', args, f_num, f_size, padding, strides, True, int(b_mrg), name=name, cfg=para, io_shape=io_shape, cfg_o=cfg_o)
    _dict = {'W': W_conv, 'b': b_conv, 'M_m_mul': M_m_mul, 'M_m_sft': M_m_sft, 'M_b_mul': M_b_mul, 'M_b_sft': M_b_sft, 'l_M_sk_mul': [], 'l_M_sk_sft': [], 'para': para, 'name': name}
    logging.info('{} > in_shape:{} ; out_shape:{}'.format(name, tensor_in.shape, h_conv.shape))
    l_tensor.append(h_conv)
    l_d_var.append(_dict)
