import tensorflow as tf
import numpy as np
import os
import logging

import util
import time
import CNN_ctor

en_training = True
en_trace = False
n_step_inf = 12000#init.n_cycle

#if __name__ == '__main__':
def main(args):
    my_file = args.path_output_file
    my_net = args.path_saved_net
    txt_data = args.path_data_txt
    Dt = args.DNN_model

    str_f0 = 'fpg_{}'.format(Dt)
    str_f = 'trg_{}_w{}a{}'.format(Dt, args.w_qn_bits, args.a_qn_bits)
    init = args.init
    B = args.n_batch_inf
    if os.path.isdir('{}/{}_train{:0>6d}'.format(my_net, str_f, init.n_cycle_tr)):
        return

    time1 = time.time()
    tf.reset_default_graph()

    # define placeholder for inputs to network
    xs = tf.placeholder(tf.float32, [None, init.dataset.X0 * init.dataset.Y0 * init.dataset.C]) # 32x32
    ifmap = tf.transpose(tf.reshape(xs / 128. - 1, [-1, init.dataset.C, init.dataset.X0, init.dataset.Y0]), [0, 2, 3, 1])
    ys = tf.placeholder(tf.float32, [None, init.dataset.K])
    p_keep_in = tf.placeholder(tf.float32)
    p_keep_pool = tf.placeholder(tf.float32)
    learning_rate = tf.placeholder(tf.float32)

    global_step = tf.placeholder(tf.int32)
    ema = tf.train.ExponentialMovingAverage(decay=0.999, num_updates=global_step)

    l_ofmap = [ifmap]
    l_p_keep = [p_keep_in, p_keep_pool]
    l_ema_op = []
    l_d_var = []
    tuple_args = ('retrain', l_ofmap, l_d_var, args, ema, l_ema_op)

    CNN_ctor.def_Net(l_p_keep, tuple_args)
    
    tensor_final = l_ofmap[-1]

    if en_training:
        #train_step = util.train_step_momentum(args, tensor_final, ys, global_step)
        train_step, v_lr, v_rs0 = util.train_step_momentum(args, tensor_final, ys)
        with tf.control_dependencies([train_step]):
            train_step = tf.group(*l_ema_op)
            
        if not os.path.isdir(my_file):
            os.mkdir(my_file)
        if not os.path.isdir(my_net):
            os.mkdir(my_net)
        if not os.path.isdir(txt_data):
            os.mkdir(txt_data)
        if not os.path.isdir('{}/{}'.format(txt_data, Dt)):
            os.mkdir('{}/{}'.format(txt_data, Dt))
        if not os.path.isdir('{}/{}/test_file_w{}a{}'.format(txt_data, Dt, args.w_qn_bits, args.a_qn_bits)):
            os.mkdir('{}/{}/test_file_w{}a{}'.format(txt_data, Dt, args.w_qn_bits, args.a_qn_bits))
        file = open('{}/{}_tr.csv'.format(my_file, str_f),'a')
    else:
        file = open('{}/{}_inf.csv'.format(my_file, str_f),'w')

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    config.gpu_options.per_process_gpu_memory_fraction=0.5
    os.environ['CUDA_VISIBLE_DEVICES'] = str(args.gpu)
    sess = tf.Session(config=config)
    sess.run(tf.global_variables_initializer())
    saver_all = tf.train.Saver(max_to_keep=100)

    l_var_param = []
    for _dict in l_d_var:
        if _dict == None or _dict['name'][-3:] == 'Avg':
            continue
        l_var_param.append(_dict['W'])
        l_var_param.append(_dict['b'])
        l_var_param.append(_dict['beta'])
        l_var_param.append(_dict['gamma'])
        l_var_param.append(_dict['min_act_ema'])
        l_var_param.append(_dict['max_act_ema'])
    saver_param = tf.train.Saver(l_var_param, max_to_keep=100)

    if en_training:
        i_th = util.get_saved_steps(my_net+'/', str_f)
        if i_th >= 0:
            i_init = i_th+1
            saver_all.restore(sess, '{}/{}_train{:0>6d}/ckpt'.format(my_net, str_f, i_th))
            i_th = int(i_th*1.1)
        else:
            i_th = 100
            i_init = 1
            saver_param.restore(sess, '{}/{}_param_train{:0>6d}/ckpt'.format(my_net, str_f0, init.n_cycle_fp))#test
        for i in range(i_init, init.n_cycle_tr+1):
            batch_xs, batch_ys = init.next_batch()
            sess.run(train_step, feed_dict={xs: batch_xs, ys: batch_ys, p_keep_in: 0.8, p_keep_pool: 0.5, global_step: i})
            if i == i_th or i == init.n_cycle_tr:
                rs1 = util.inf_acc(args, sess, tensor_final, xs, ys, p_keep_in, p_keep_pool)
                a_lr = v_lr.eval(sess)
                rs0 = v_rs0.eval(sess)
                logging.info('INFO: #{} accuracy:{} lr:{}'.format(i, rs1, a_lr))
                file.write('{},{},{}\n'.format(i, rs1, a_lr))
                if rs0 != None and i >= 1000 and rs1 <= (1-0.02*(1000/i))*rs0:
                    v_lr.load(a_lr * args.lr_decay_rate, sess)
                if rs1 > rs0:
                    v_rs0.load(rs1, sess)
                    if not en_trace:
                        os.system('rm -r {}/{}_train*'.format(my_net, str_f))
                saver_all.save(sess, '{}/{}_train{:0>6d}/ckpt'.format(my_net, str_f, i), write_meta_graph=False)
                i_th = int(i_th*1.1)
        file.close()
        
        logging.info('INFO: Dump filter...')
        with open('{}/{}/test_file_w{}a{}/filter.txt'.format(txt_data, Dt, args.w_qn_bits, args.a_qn_bits), 'w') as fout:
            S_in = 1/128
            l_S_in = [S_in]
            lim_avg = 2**args.w_qn_bits # 256
            for _dict in l_d_var:
                if _dict == None:
                    fout.write('Avg_Pooling:size\n0\n')
                    continue
                name = _dict['name']
                l_M_sk = []
                if name[-3:] == 'Avg':
                    a_W_qn = (sess.run(_dict['W']) * lim_avg).astype(np.int8)
                    M_m = (512, -9)
                    a_b_qn = np.zeros(int(_dict['W'].shape[2]), dtype=np.int16)
                    M_b = (512, -9 - args.w_qn_bits)
                else:
                    # ----- Weight setting ----- #
                    W_m = _dict['W'] * _dict['gamma'] / (_dict['sigma_ema'] + args.epsilon_bn)
                    abs_max_W = util.get_abs_max(W_m)
                    W_qn = util.cast(args, W_m, abs_max_W, 'W')
                    a_W_qn = sess.run(W_qn)
                    # ----- bias setting ----- #
                    b_m = (_dict['b'] - _dict['mu_ema']) * _dict['gamma'] / (_dict['sigma_ema'] + args.epsilon_bn) + _dict['beta']
                    abs_max_b = util.get_abs_max(b_m)
                    S_m = abs_max_W / init.int_abs_max
                    S_b = abs_max_b / init.int_abs_max_b
                    M_m = util.get_mul_sft(sess, S_in * S_m / S_b)
                    b_qn = util.cast(args, b_m, abs_max_b, 'b')
                    a_b_qn = sess.run(b_qn)
                    S_in = _dict['abs_max_act'] / init.int_abs_max_a
                    M_b = util.get_mul_sft(sess, S_b / S_in)

                    l_S_in.append(S_in)
                # ----- export the params ----- #
                util.dump(fout, a_W_qn, 'Weignt_{}'.format(name))
                fout.write('Weignt_{}:mul\n'.format(name))
                fout.write('{} {}\n'.format(*M_m))
                util.dump(fout, a_b_qn, 'bias_{}'.format(name))
                fout.write('bias_{}:mul\n'.format(name))
                fout.write('{} {}\n'.format(*M_b))

                logging.info(name)
            #fout.write('Avg_Pooling:size\n0\n')
        
    else:
        i = 10
        while True:
            if not en_trace or i > n_step_inf:
                i = n_step_inf
            saver_all.restore(sess, my_net+'/{}_train{:0>6d}/ckpt'.format(str_f, i))
            rs1 = util.inf_acc(args, sess, tensor_final, xs, ys, p_keep_in, p_keep_pool)
            logging.info('INFO: #{} accuracy:{}'.format(i, rs1))
            file.write('{},{}\n'.format(i, rs1))
            if i == n_step_inf:
                break
            i = int(i*1.1)
        file.close()

    time2 = time.time()
    logging.info('Timing: {}'.format(time2-time1))
