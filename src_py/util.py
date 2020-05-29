import tensorflow as tf
import numpy as np
import logging
import os

def get_saved_steps(dir_, name):
    num_max = -1
    id_name = name + '_train'
    for st_file in os.listdir(dir_):
        if id_name not in st_file:
            continue
        num_ = int(st_file[-6:])
        if num_ > num_max:
            num_max = num_
    return num_max

def inf_acc(args, sess, model_out, xs, ys, p_keep_in=None, p_keep_pool=None, n_batch=None):
    init = args.init
    correct_prediction = tf.equal(tf.argmax(model_out,1), tf.argmax(ys,1))
    acc = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))
    if n_batch != None:
        if n_batch <= 0:
            logging.info('util.py/inf_acc: Batch size must be bigger than 0!!')
            sys.exit(9487)
        if n_batch > init.dataset.test_images_len():
            logging.info('util.py/inf_acc: Batch size must not be bigger than dataset size!!')
            sys.exit(9487)
        res_batch = n_batch
    else:
        res_batch = init.dataset.test_images_len()
    if p_keep_in == None:
        feed_dict = {}
    else:
        feed_dict = {p_keep_in: 1, p_keep_pool: 1}
    psum = 0
    while res_batch > 0:
        inf_img, inf_lab = init.dataset.test_next_batch(100)
        feed_dict[xs], feed_dict[ys] = inf_img, inf_lab
        acc_value = sess.run(acc, feed_dict=feed_dict) * len(inf_img)
        #logging.info('accuracy for residue ', res_batch, ': ', acc_value)
        psum += acc_value
        res_batch -= len(inf_img)
    if n_batch != None:
        return psum / n_batch
    else:
        return psum / init.dataset.test_images_len()
'''
def train_step_momentum(args, tensor_train, ys, global_step):
    init = args.init
    h_softmax = tf.nn.softmax(tensor_train, name='softmax')
    cross_entropy = tf.reduce_mean(-tf.reduce_sum(ys * tf.log(h_softmax),
                                                  reduction_indices=[1]), name='cross_entropy') # loss
    lr = tf.train.exponential_decay(args.learning_rate_max,
                                    global_step,
                                    init.half_decay_cycle,
                                    0.5)
    train_step = tf.train.MomentumOptimizer(lr, args.momentum).minimize(cross_entropy)
    return train_step
'''
def train_step_momentum(args, tensor_train, ys):
    h_softmax = tf.nn.softmax(tensor_train, name='softmax')
    cross_entropy = tf.reduce_mean(-tf.reduce_sum(ys * tf.log(h_softmax),
                                                  reduction_indices=[1]), name='cross_entropy') # loss
    lr = tf.Variable(args.learning_rate_max, trainable=False, dtype=tf.float32)
    rs0 = tf.Variable(0, trainable=False, dtype=tf.float32)
    train_step = tf.train.MomentumOptimizer(lr, args.momentum).minimize(cross_entropy)
    return train_step, lr, rs0

def train_step_adam(args, tensor_train, ys, global_step):
    init = args.init
    h_softmax = tf.nn.softmax(tensor_train, name='softmax')
    cross_entropy = tf.reduce_mean(-tf.reduce_sum(ys * tf.log(h_softmax),
                                                  reduction_indices=[1]), name='cross_entropy') # loss

    lr = tf.train.exponential_decay(args.learning_rate_max,
                                    global_step,
                                    init.half_decay_cycle,
                                    0.5)
    train_step = tf.train.AdamOptimizer(lr).minimize(cross_entropy)
    return train_step

def dump(fout, array, name):
    fout.write('{}:size\n'.format(name))
    for _x in array.shape:
        fout.write('{} '.format(_x))
    fout.write('\n{}:value\n'.format(name))
    if len(array.shape) == 4:
        for _x0 in array:
            for _x1 in _x0:
                for _x2 in _x1:
                    for _x3 in _x2:
                        fout.write('{}\n'.format(_x3))
    if len(array.shape) == 2:
        for _x0 in array:
            for _x1 in _x0:
                fout.write('{}\n'.format(_x1))
    elif len(array.shape) == 1:
        for _x0 in array:
            fout.write('{}\n'.format(_x0))

def proc_var(array, ratio_mu, ratio_sigma):
    a_normal = np.random.normal(1 + ratio_mu, ratio_sigma, array.shape)
    array_out = np.clip(array * a_normal, -127, 127)
    return array_out.astype(array.dtype)

def fetch(fin):
    str_line = fin.readline() # size
    list_sp = fin.readline().split()
    shape = []
    for _x in list_sp:
        shape.append(int(_x))
    if len(shape) == 1 and shape[0] == 0:
        return
    #logging.info('size:', shape) #debug
    array = np.zeros(shape=shape, dtype=np.int32)
    str_line = fin.readline() # value
    if len(shape) == 4:
        for _x0 in range(shape[0]):
            for _x1 in range(shape[1]):
                for _x2 in range(shape[2]):
                    for _x3 in range(shape[3]):
                        array[_x0, _x1, _x2, _x3] = int(fin.readline()[:-1])
    elif len(shape) == 3:
        for _x0 in range(shape[0]):
            for _x1 in range(shape[1]):
                for _x2 in range(shape[2]):
                    array[_x0, _x1, _x2] = int(fin.readline()[:-1])
    elif len(shape) == 2:
        for _x0 in range(shape[0]):
            for _x1 in range(shape[1]):
                array[_x0, _x1] = int(fin.readline()[:-1])
    elif len(shape) == 1:
        for _x0 in range(shape[0]):
            array[_x0] = int(fin.readline()[:-1])
    else:
        logging.info('Error: inf_graph.py/fatch: invalid shape!')
        exit(9487)
    str_line = fin.readline() # mul
    list_sp = fin.readline().split()
    if list_sp == []:
        mul = (0, 0)
    else:
        #logging.info('mul:', list_sp) #debug
        mul = (int(list_sp[0]), int(list_sp[1]))
    return array, mul

def get_abs_max(tensor_in):
    min_ = tf.math.reduce_min(tensor_in)
    max_ = tf.math.reduce_max(tensor_in)
    abs_max_ = tf.math.maximum(tf.math.abs(min_), tf.math.abs(max_))
    return abs_max_

def get_mul_sft(sess, tensor_in):
    num_in = sess.run(tensor_in)
    mul = num_in
    sft = 0
    while(mul < 512):
        mul *= 2
        sft -= 1
    while(mul >= 1024):
        mul /= 2
        sft += 1
    return (int(mul), sft)

def cast(args, tensor_in, tensor_abs_max, type):
    init = args.init
    if type == 'W':
        int_max_ = init.int_abs_max
        dtype = init.dtype_W
    elif type == 'b':
        int_max_ = init.int_abs_max_b
        dtype = init.dtype_b
    elif type == 'act':
        int_max_ = init.int_abs_max_a
        dtype = init.dtype_W
    tmp = tensor_in * int_max_ / tensor_abs_max
    tmp = tf.minimum(tmp, int_max_)
    tmp = tf.maximum(tmp, -int_max_)
    return tf.cast(tf.round(tmp), dtype=dtype)

def get_ofmap(args, layer_final, xs, ys, sess):
    init = args.init
    res_batch = init.dataset.test_images_len()
    #res_batch = 10#e
    array = np.zeros([0, int(layer_final.shape[1])], dtype=np.int8)
    #array = np.zeros([0, int(layer_final.shape[1]), int(layer_final.shape[2]), int(layer_final.shape[3])], dtype=np.int8)#e
    psum = 0
    while res_batch > 0:
        inf_img, inf_lab = init.dataset.test_next_batch(100)#100
        array_seg = sess.run(layer_final, feed_dict={xs: inf_img, ys: inf_lab})# * len(inf_img)
        #logging.info('array_seg for residue ', res_batch, ': ', array_seg)
        array = np.concatenate((array, array_seg), axis=0)
        res_batch -= len(inf_img)
    return array

def file_ctor_CNN(type, args, f_num=None, f_size=None, padding=None, strides=None, relu=None, b_mrg=None, name=None, cfg=None, cfg_o=None, io_shape=None):
    with open('src_sc/CNN_ctor.cpp', 'a') as fin:
        with open('cfg_dt.csv', 'a') as fin1:
            C, XY0, XY = io_shape
            if type == 'global_avg':
                fin.write('add_avgpooling_layer({0}, \"{1}\");\n'.format(b_mrg, name))
                fin1.write('{0},avg,{2},{2},{1},{1},{2},{2},{2},{3},{3},{4}\n'.format(name, C, XY0, XY, b_mrg))
            elif type == 'conv':
                if cfg == 'p1':
                    str_cfg = 'none'
                elif strides == 1:
                    if cfg == 'p2':
                        str_cfg = 'pt_2Y'
                    elif cfg == 'p4':
                        str_cfg = 'pt_2XY'
                elif strides == 2:
                    if cfg_o == 'p1':
                        if cfg == 'p2':
                            str_cfg = 'pt_2YS'
                        elif cfg =='p4':
                            str_cfg = 'pt_2XYS'
                    else:
                        if cfg == 'p2':
                            str_cfg = 'pt_2Ym'
                        elif cfg == 'p4':
                            str_cfg = 'pt_2XYm'
                padding_sc = 'true' if padding == 'SAME' else 'false'
                relu_sc = 'true' if relu else 'false'

                if f_num == None:
                    fin.write('add_DWconv_layer({0}, {0}, {1}, {2}, {3}, {4}, \"{5}\", Parallel_type::{6});\n'.format(f_size, padding_sc, strides, relu_sc, b_mrg, name, str_cfg))
                    fin1.write('{0},{1},{2},{2},{3},{3},{4},{5},{5},{6},{6},{7}\n'.format(name, str_cfg, f_size, C, strides, XY0, XY, b_mrg))
                else:
                    fin.write('add_conv_layer({0}, {1}, {1}, {2}, {3}, {4}, {5}, \"{6}\", Parallel_type::{7});\n'.format(f_num, f_size, padding_sc, strides, relu_sc, b_mrg, name, str_cfg))
                    fin1.write('{0},{1},{2},{2},{3},{4},{5},{6},{6},{7},{7},{8}\n'.format(name, str_cfg, f_size, C, f_num, strides, XY0, XY, b_mrg))

def file_ctor_CFG(args, file_csv, cycle_padi=1, cycle_pado=1, cycle_act=1, cycle_pool=1):
    dir_txt = '{}/{}/test_file_w{}a{}'.format(args.path_data_txt, args.DNN_model, args.w_qn_bits, args.a_qn_bits)
    cycle_conv = args.w_qn_bits * args.a_qn_bits if args.RRAM_array else 1
    with open('{}/config_seg1.h'.format(args.path_sc_config), 'w') as fout:
        fout.write('#ifndef _CONFIG_H_\n#define _CONFIG_H_\n\n')
        fout.write('#define IFMAP_TXT "{}/image.txt"\n\n'.format(args.path_dataset_txt))
        fout.write('#define FILTER_TXT "{}/filter.txt"\n'.format(dir_txt))
        fout.write('#define OFMAP_TXT "{}/out.txt"\n'.format(dir_txt))
        fout.write('#define CYCLE_CNT_CSV "{}"\n\n'.format(file_csv))
        fout.write('#define CLK_CYCLE {} // cycle time = 10 ns\n'.format(cycle_conv))
        fout.write('#define PADDING_IN_CYCLE {}\n'.format(cycle_padi))
        fout.write('#define PADDING_OUT_CYCLE {}\n'.format(cycle_pado))
        fout.write('#define ACT_CYCLE {}\n'.format(cycle_act))
        fout.write('#define POOL_CYCLE {}\n\n'.format(cycle_pool))
        #fout.write('#define IN_WIDTH {}//8, act\n'.format(args.a_qn_bits))
        fout.write('#define IN_WIDTH 8\n')
        fout.write('#define WEIGHT_WIDTH {}\n'.format(args.w_qn_bits))
        fout.write('#define BIAS_WIDTH {}\n'.format(args.b_qn_bits))
        fout.write('#define mul_C_in {}\n'.format(args.mul_C))

def label_accuracy(args, file_csv):
    file_out = '{}/{}/test_file_w{}a{}/out.txt'.format(args.path_data_txt, args.DNN_model, args.w_qn_bits, args.a_qn_bits)
    file_out_ref = '{}/{}/test_file_w{}a{}/out_ref.txt'.format(args.path_data_txt, args.DNN_model, args.w_qn_bits, args.a_qn_bits)
    logging.info('INFO: Fetch label...')
    with open('{}/label.txt'.format(args.path_dataset_txt),'r') as fin:#10k
        a_label, _ = fetch(fin)
    logging.info('INFO: Fetch output reference data...')
    with open(file_out_ref,'r') as fin:
        a_out_ref, _ = fetch(fin)
    with open(file_out,'r') as fin:
        a_out, _ = fetch(fin)

    a_label = np.argmax(a_label, axis=1)
    a_out_ref = np.argmax(a_out_ref, axis=1)
    a_out = np.argmax(a_out, axis=1)
    acc = np.sum(a_label == a_out) / len(a_out)
    acc_ref = np.sum(a_out_ref == a_out) / len(a_out)

    with open(file_csv, 'a') as fout:
        fout.write('classification accuracy,{}%\n'.format(acc*100))
        fout.write('reference accuracy,{}%\n'.format(acc_ref*100))
    logging.info('INFO: classification accuracy: {:>6.2f} %'.format(acc*100))
    logging.info('INFO: reference accuracy: {:>6.2f} %'.format(acc_ref*100))
