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
    str_f = 'fpg_{}'.format(args.DNN_model)
    init = args.init
    if os.path.isdir('{}/{}_train{:0>6d}'.format(my_net, str_f, init.n_cycle_fp)):
        return

    time1 = time.time()
    tf.reset_default_graph()

    # define placeholder for inputs to network
    xs = tf.placeholder(tf.float32, [None, init.dataset.X0 * init.dataset.Y0 * init.dataset.C]) # 32x32
    ifmap = tf.transpose(tf.reshape(xs / 128. - 1, [-1, init.dataset.C, init.dataset.X0, init.dataset.Y0]), [0, 2, 3, 1])
    #print('INFO: ifmap:', ifmap)
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
    tuple_args = ('pretrain', l_ofmap, l_d_var, args, ema, l_ema_op)

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
        file = open(my_file+'/{}_tr.csv'.format(str_f),'a')
    else:
        file = open(my_file+'/{}_inf.csv'.format(str_f),'w')

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
        for i in range(i_init, init.n_cycle_fp+1):
            batch_xs, batch_ys = init.next_batch()
            sess.run(train_step, feed_dict={xs: batch_xs, ys: batch_ys, p_keep_in: 0.8, p_keep_pool: 0.5, global_step: i})
            if i == i_th or i == init.n_cycle_fp:
                rs1 = util.inf_acc(args, sess, tensor_final, xs, ys, p_keep_in, p_keep_pool)
                a_lr = v_lr.eval(sess)
                rs0 = v_rs0.eval(sess)
                logging.info('INFO: #{} accuracy:{} lr:{}'.format(i, rs1, a_lr))
                file.write('{},{},{}\n'.format(i, rs1, a_lr))
                if rs0 != None and i >= 5000 and rs1 <= (1-0.04*(5000/i))*rs0:
                    v_lr.load(a_lr * args.lr_decay_rate, sess)
                if rs1 > rs0:
                    v_rs0.load(rs1, sess)
                    if not en_trace:
                        os.system('rm -r {}/{}_train*'.format(my_net, str_f))
                        os.system('rm -r {}/{}_param_train*'.format(my_net, str_f))
                saver_all.save(sess, '{}/{}_train{:0>6d}/ckpt'.format(my_net, str_f, i), write_meta_graph=False)
                saver_param.save(sess, '{}/{}_param_train{:0>6d}/ckpt'.format(my_net, str_f, i), write_meta_graph=False)
                i_th = int(i_th*1.1)
        file.close()
        with open('{}/{}_dist.csv'.format(my_file, str_f) ,'w') as f_dist:
            f_dist.write(',mean_ori,std_ori,mean_mrg,std_mrg\n')
            #feed_dict={xs: init.dataset.test_images, ys: init.dataset.test_labels, p_keep_in: 1, p_keep_pool: 1}
            for _dict in l_d_var:
                if _dict == None or _dict['name'][-3:] == 'Avg':
                    continue
                name = _dict['name']
                # ----- Weight ----- #
                mean_W_o, var_W_o = tf.nn.moments(_dict['W'], axes=[0,1,2,3])
                std_W_o = tf.sqrt(var_W_o)
                f_dist.write('W_conv_{},{},{},'.format(name, sess.run(mean_W_o), sess.run(std_W_o)))
                W_m = _dict['W'] * _dict['gamma'] / (_dict['sigma_ema'] + args.epsilon_bn)
                mean_W_m, var_W_m = tf.nn.moments(W_m, axes=[0,1,2,3])
                std_W_m = tf.sqrt(var_W_m)
                f_dist.write('{},{}\n'.format(sess.run(mean_W_m), sess.run(std_W_m)))
                # ----- bias ----- #
                mean_b_o, var_b_o = tf.nn.moments(_dict['b'], axes=[0])
                std_b_o = tf.sqrt(var_b_o)
                f_dist.write('b_conv_{},{},{},'.format(name, sess.run(mean_b_o), sess.run(std_b_o)))
                b_m = (_dict['b'] - _dict['mu_ema']) * _dict['gamma'] / (_dict['sigma_ema'] + args.epsilon_bn) + _dict['beta']
                mean_b_m, var_b_m = tf.nn.moments(b_m, axes=[0])
                std_b_m = tf.sqrt(var_b_m)
                f_dist.write('{},{}\n'.format(sess.run(mean_b_m), sess.run(std_b_m)))
                logging.info(name)
    else:
        i = 10
        while True:
            if not en_trace or i > n_step_inf:
                i = n_step_inf
            saver_all.restore(sess, '{}/{}_train{:0>6d}/ckpt'.format(my_net, str_f, i))
            #smy = sess.run(merged, feed_dict={xs: init.inf_img, ys: init.inf_lab, p_keep_in: 1, p_keep_pool: 1})
            #writer.add_summary(smy, i)
            rs1 = util.inf_acc(args, sess, tensor_final, xs, ys, p_keep_in, p_keep_pool)
            logging.info('#{} accuracy:{}'.format(i, rs1))
            file.write('{},{}\n'.format(i, rs1))
            if i == n_step_inf:
                break
            i = int(i*1.1)
        file.close()

    time2 = time.time()
    logging.info('Timing: {}'.format(time2-time1))
