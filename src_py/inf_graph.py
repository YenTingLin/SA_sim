import tensorflow as tf
import numpy as np
import os
import logging

import util
import time
import CNN_ctor

#if __name__ == '__main__':
def main(args):

    my_file = args.path_output_file
    txt_data = args.path_data_txt
    Dt = args.DNN_model

    str_f = 'infg_{}_w{}a{}'.format(Dt, args.w_qn_bits, args.a_qn_bits)
    init = args.init

    time1 = time.time()
    tf.reset_default_graph()

    # define placeholder for inputs to network
    xs = tf.placeholder(tf.uint8, [None, init.dataset.X0 * init.dataset.Y0 * init.dataset.C]) # 32x32
    ifmap = tf.cast(xs -128, dtype=tf.int8)
    ifmap = tf.transpose(tf.reshape(ifmap, [-1, init.dataset.C, init.dataset.X0, init.dataset.Y0]), [0, 2, 3, 1])
    ys = tf.placeholder(tf.int8, [None, init.dataset.K])
    p_keep_in = tf.placeholder(tf.float32)
    p_keep_pool = tf.placeholder(tf.float32)

    l_ofmap = [ifmap]
    l_p_keep = [p_keep_in, p_keep_pool]
    l_d_var = []
    tuple_args = ('inference', l_ofmap, l_d_var, args, None, None)

    with open('src_sc/CNN_ctor.cpp', 'w') as fin:
        fin.write('#include \"CNN.h\"\n\n')
        fin.write('void CNN::def_CNN(){\n')
    with open('cfg_dt.csv', 'w') as fin:
        fin.write('name,type,I,J,C,K,St,X0,Y0,X,Y,mrg_bit\n')
    CNN_ctor.def_Net(l_p_keep, tuple_args)
    with open('src_sc/CNN_ctor.cpp', 'a') as fin:
        fin.write('}\n')

    if not args.export_txt:
        tf.reset_default_graph()
        return

    layer_final = l_ofmap[-1]

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    config.gpu_options.per_process_gpu_memory_fraction=0.5
    os.environ['CUDA_VISIBLE_DEVICES'] = str(args.gpu)
    sess = tf.Session(config=config)
    sess.run(tf.global_variables_initializer())

    logging.info('INFO: Fetch filter...')
    with open('{}/{}/test_file_w{}a{}/filter.txt'.format(txt_data, Dt, args.w_qn_bits, args.a_qn_bits),'r') as fin:
        for _dict in l_d_var:
            if _dict == None:
                util.fetch(fin)
                continue
            name = _dict['name']
            a_W, M_m = util.fetch(fin)
            _dict['W'].load(a_W, sess)
            _dict['M_m_mul'].load(M_m[0], sess)
            _dict['M_m_sft'].load(M_m[1], sess)
            a_b, M_b = util.fetch(fin)
            _dict['b'].load(a_b, sess)
            _dict['M_b_mul'].load(M_b[0], sess)
            _dict['M_b_sft'].load(M_b[1], sess)

            logging.info(name)
    
    with open('{}/{}_inf.csv'.format(my_file, str_f),'a') as file:
        rs1 = util.inf_acc(args, sess, layer_final, xs, ys, p_keep_in, p_keep_pool)
        logging.info('accuracy:{}'.format(rs1))
        file.write('accuracy,{}\n'.format(rs1))
    
    logging.info('INFO: Fetch image...')
    with open('{}/image.txt'.format(args.path_dataset_txt),'r') as fin:
        a_image, _ = util.fetch(fin)
        a_image = np.reshape(np.transpose(a_image, [0, 3, 1, 2]), [-1, init.dataset.C * init.dataset.X0 * init.dataset.Y0])
    logging.info('INFO: Dump output data...')
    with open('{}/{}/test_file_w{}a{}/out_ref.txt'.format(txt_data, Dt, args.w_qn_bits, args.a_qn_bits), 'w') as fout:
        n_res = len(a_image)
        n_pos = 0
        l_a_out = []
        while n_res > 0:
            if n_res > 100:
                a_out = sess.run(layer_final, feed_dict={xs: a_image[n_pos : n_pos+100], p_keep_in: 1, p_keep_pool: 1})
            else:
                a_out = sess.run(layer_final, feed_dict={xs: a_image[n_pos : ], p_keep_in: 1, p_keep_pool: 1})
            l_a_out.append(a_out)
            n_res -= 100
            n_pos += 100
        a_out = np.concatenate(l_a_out, axis=0)
        util.dump(fout, a_out, 'ofmap')
        #util.dump_1(fout, a_ofmap[:10], 'ofmap')
        #logging.info(a_out)
    '''
    inf_img, inf_lab = init.dataset.test_next_batch(1)
    array_seg = sess.run(layer1, feed_dict={xs: inf_img, ys: inf_lab})
    logging.info(array_seg)
    '''
    time2 = time.time()
    logging.info('Timing: {}'.format(time2-time1))

