import os
import numpy as np
import argparse

parser = argparse.ArgumentParser('SAsim')
parser.add_argument('--DNN_model', type=str, default='TheAllConvNet', help='')#'TheAllConvNet'
parser.add_argument('--layer_cfg', type=str, default='cp1', help='')
parser.add_argument('--w_qn_bits', type=int, default=7, help='weight quantization bits') # 8 -> 3
parser.add_argument('--b_qn_bits', type=int, default=-1, help='bias quantization bits, -1 for w_qn_bits + 8')
parser.add_argument('--a_qn_bits', type=int, default=7, help='activation quantization bits') # 8 -> 3

args = parser.parse_args()

if args.b_qn_bits == -1:
    args.b_qn_bits = args.w_qn_bits + 8

proc_syn = 130 #check
proc_NeuroSim = 32
sc_area = (proc_NeuroSim/proc_syn)**2

def syn_PE(dict_, p1_weight, p1_act, p1_psum):
    key = '{}_{}_{}'.format(p1_weight, p1_act, p1_psum)
    if key in dict_:
        return dict_[key]
    else:
        print('syn_PE/cfg: ({:2d}, {:2d}, {:2d})'.format(p1_weight, p1_act, p1_psum))
        with open('config.v','w') as f1:
            f1.write('`timescale 1ns / 100ps\n\n')
            f1.write('`define p1_weight {}\n'.format(p1_weight))
            f1.write('`define p1_act {}\n'.format(p1_act))
            f1.write('`define p1_psum {}\n\n'.format(p1_psum))
        os.system('dc_shell -f p1_PE.tcl')
        for str_2 in open('area_p1_PE.txt','r'):
            if 'Total cell area:' in str_2:
                area_o = float(str_2.split()[-1])
                break
        os.system('mv area_p1_PE.txt my_file/area_p1_PE_w{}a{}m{}.txt'.format(p1_weight, p1_act, p1_psum))
        with open('log_p1.csv', 'a') as f2:
            f2.write('{},{},{},{}\n'.format(p1_weight, p1_act, p1_psum, area_o))
        area = area_o * sc_area
        dict_[key] = area
        return area

def syn_fifo(dict_, Nr, p2_L, p2_W):
    p2_L, p2_W = int(p2_L), int(p2_W)
    key = '{}_{}'.format(p2_L, p2_W)
    if Nr * p2_L * p2_W == 0:
        return 0.
    elif key in dict_:
        return Nr * dict_[key]
    else:
        print('syn_fifo/cfg: ({:5d}, {:5d})'.format(p2_L, p2_W))
        with open('config.v','w') as f1:
            f1.write('`timescale 1ns / 100ps\n\n')
            f1.write('`define p2_size {} //Y_0 extension, Lreg (# elements)\n'.format(p2_L))
            f1.write('`define p2_width {} //for ifmap, Wreg (# bits)\n'.format(p2_W))
        os.system('dc_shell -f p2_fifo.tcl')
        for str_2 in open('area_p2_fifo.txt','r'):
            if 'Total cell area:' in str_2:
                area_o = float(str_2.split()[-1])
                break
        os.system('mv area_p2_fifo.txt my_file/area_p2_fifo_{:0>2d}_{:0>5d}.txt'.format(p2_L, p2_W))
        with open('log_p2.csv', 'a') as f2:
            f2.write('{},{},{}\n'.format(p2_L, p2_W, area_o))
        area = area_o * sc_area
        dict_[key] = area
        return Nr * area

def syn_avg(dict_, p3_act):
    key = '{}'.format(p3_act)
    if key in dict_:
        return dict_[key]
    else:
        print('syn_avg/cfg: ({:2d})'.format(p3_act))
        with open('config.v','w') as f1:
            f1.write('`timescale 1ns / 100ps\n\n')
            f1.write('`define p3_cycle 64\n')
            f1.write('`define p3_act {}\n\n'.format(p3_act))
        os.system('dc_shell -f p3_avg.tcl')
        for str_2 in open('area_p3_avg.txt','r'):
            if 'Total cell area:' in str_2:
                area_o = float(str_2.split()[-1])
                break
        os.system('mv area_p3_avg.txt my_file/area_p3_avg_a{}.txt'.format(p3_act))
        with open('log_p3.csv', 'a') as f2:
            f2.write('{},{}\n'.format(p3_act, area_o))
        area = area_o * sc_area
        dict_[key] = area
        return area

def syn_act(dict_, p4_bias, p4_act):
    key = '{}_{}'.format(p4_bias, p4_act)
    if key in dict_:
        return dict_[key]
    else:
        print('syn_act/cfg: ({:2d}, {:2d})'.format(p4_bias, p4_act))
        with open('config.v','w') as f1:
            f1.write('`timescale 1ns / 100ps\n\n')
            f1.write('`define p4_bias {}\n'.format(p4_bias))
            f1.write('`define p4_act {}\n\n'.format(p4_act))
        os.system('dc_shell -f p4_act.tcl')
        for str_2 in open('area_p4_act.txt','r'):
            if 'Total cell area:' in str_2:
                area_o = float(str_2.split()[-1])
                break
        os.system('mv area_p4_act.txt my_file/area_p4_act_b{}a{}.txt'.format(p4_bias, p4_act))
        with open('log_p4.csv', 'a') as f2:
            f2.write('{},{},{}\n'.format(p4_bias, p4_act, area_o))
        area = area_o * sc_area
        dict_[key] = area
        return area

if __name__ == '__main__':
    d_PE = {} # { p1_weight }_{ p1_act }_{ p1_psum }
    d_fifo = {} # { p2_L }_{ p2_W }
    d_avg = {} # { p3_act }
    d_act = {} # { p4_bias }_{ p4_act }
    #d_skmul = {} # { p5_act }
    #d_RRAM = {} # { C }_{ K }_{ dw }_{ cell }

    if not os.path.isdir('my_file/'):
        os.mkdir('my_file/')
    if not os.path.isfile('log_p1.csv'):
        with open('log_p1.csv','w') as f1:
            f1.write('p1_weight,p1_act,p1_psum,area\n')
    if not os.path.isfile('log_p2.csv'):
        with open('log_p2.csv','w') as f1:
            f1.write('p2_L,p2_W,area\n')
    if not os.path.isfile('log_p3.csv'):
        with open('log_p3.csv','w') as f1:
            f1.write('p3_act,area\n')
    if not os.path.isfile('log_p4.csv'):
        with open('log_p4.csv','w') as f1:
            f1.write('p4_bias,p4_act,area\n')

    for st_line in open('log_p1.csv', 'r'):
        if 'p1' in st_line:
            continue
        l_st_seg = st_line.split(',')
        d_PE['{}_{}_{}'.format(*l_st_seg[:3])] = float(l_st_seg[3]) * sc_area
    for st_line in open('log_p2.csv', 'r'):
        if 'p2' in st_line:
            continue
        l_st_seg = st_line.split(',')
        d_fifo['{}_{}'.format(*l_st_seg[:2])] = float(l_st_seg[2]) * sc_area
    for st_line in open('log_p3.csv', 'r'):
        if 'p3' in st_line:
            continue
        l_st_seg = st_line.split(',')
        d_avg[l_st_seg[0]] = float(l_st_seg[1]) * sc_area
    for st_line in open('log_p4.csv', 'r'):
        if 'p4' in st_line:
            continue
        l_st_seg = st_line.split(',')
        d_act['{}_{}'.format(*l_st_seg[:2])] = float(l_st_seg[2]) * sc_area
    '''
    for st_line in open('log_p5.csv', 'r'):
        if 'p5' in st_line:
            continue
        l_st_seg = st_line.split(',')
        d_skmul[l_st_seg[0]] = float(l_st_seg[1]) * sc_area
    for st_line in open('cfg_RRAM.csv', 'r'):
        if 'C' in st_line:
            continue
        l_st_seg = st_line.split(',')
        for qn in range(8, 2, -1):
            d_RRAM['{}_{}_{}_{}b'.format(*l_st_seg[:3], qn)] = float(l_st_seg[11 - qn])
        d_RRAM['{}_{}_{}_a'.format(*l_st_seg[:3])] = float(l_st_seg[9])
    '''

    #l_layer_cfg = ['p1', 'p2', 'p4']
    #l_imp = ['digPE', 'anaRRAM', 'digRRAM']

    if not os.path.isfile('res_area.csv'):
        with open('res_area.csv', 'w') as fout2:
            fout2.write('DNN,layer cfg,weight qn bit,bias qn bit,activation qn bit,')
            fout2.write('N_PEs,Br_syn,Br_pad,N_act,N_avg,')
            fout2.write('R_PEs,Rr_syn,Rr_pad,R_act,R_avg,')
            fout2.write('total area\n')
    
    #for st_DNN in l_st_DNN:
    l_d_layer = []
    with open('cfg_dt.csv', 'r') as fin:
        st_line = fin.readline()
        l_key = st_line.split(',')
        while True:
            st_line = fin.readline()
            if st_line == '':
                break
            l_value = st_line.split(',')
            d_layer = {}
            for key, value in zip(l_key[:2], l_value[:2]):
                d_layer[key] = value
            for key, value in zip(l_key[2:-1], l_value[2:-1]):
                d_layer[key] = int(value)
            d_layer[l_key[-1][:-1]] = bool(int(l_value[-1]))
            l_d_layer.append(d_layer)

    #for layer_cfg in l_layer_cfg:
    #for imp in l_imp:
    #for qn_w in range(8, 2, -1):
    qn_w, qn_b, qn_a = args.w_qn_bits, args.b_qn_bits, args.a_qn_bits
    with open('res_{}_{}_w{}b{}a{}.csv'.format(args.DNN_model, args.layer_cfg, qn_w, qn_b, qn_a), 'w') as fout1:
        fout1.write('layer name,arch type,N_PE,A_PEs,')
        #fout1.write('Lr_sk,Wr_sk,Ar_sk,')
        fout1.write('Nr_PEi,Lr_PEi,Wr_PEi,Ar_PEi,Nr_PEj,Wr_PEj,Ar_PEj,Wr_PEo,Ar_PEo,')
        fout1.write('Wr_padi,Ar_padi,Wr_pado,Ar_pado,')
        fout1.write('N_act,A_act,N_avg,A_avg,')
        fout1.write('R_PEs,Rr_syn,Rr_pad,R_act,R_avg\n')
        a_N_acc = np.zeros([5], dtype=np.float32)
        a_A_acc = np.zeros([5], dtype=np.float32)
        for d_layer in l_d_layer:
            a_N = np.zeros_like(a_N_acc, dtype=np.float32)
            a_A = np.zeros_like(a_A_acc, dtype=np.float32)

            # ----- fetch d_layer ----- #
            _name = d_layer['name']
            _St = d_layer['St']
            '''
            _type = 'none'
            if _St == 1:
                if d_layer['type'] == 'p':
                    if layer_cfg == 'p2':
                        _type = '2Y'
                    elif layer_cfg == 'p4':
                        _type = '2XY'
            elif _St == 2:
                if d_layer['type'] == 'p':
                    if layer_cfg == 'p2':
                        _type = '2YS'
                    elif layer_cfg == 'p4':
                        _type = '2XYS'
            else:
                _type = 'avg'
            '''
            _type = d_layer['type']
            _I = d_layer['I']
            _J = d_layer['J']
            _C = d_layer['C']
            _K = d_layer['K']
            _X0 = d_layer['X0']
            _Y0 = d_layer['Y0']
            _X = d_layer['X']
            _Y = d_layer['Y']
            _mrg_bit = d_layer['mrg_bit']
            #_skip = d_layer['skip']

            # ----- layer name, arch type, N_RRAM, N_PE, A_PEs ----- #
            if _type == 'avg':
                #N_RRAM = 0
                N_PE = 0
            elif _type == 'pt_2Y':
                #N_RRAM = 2 *_I * _J
                N_PE = 2 * _I * _J * _C * _K
            elif _type == 'pt_2XY':
                #N_RRAM = 4 * _I * _J
                N_PE = 4 * _I * _J * _C * _K
            else:
                #N_RRAM = _I * _J
                N_PE = _I * _J * _C * _K
            if 'avg' in _name:
                #_dw = 1
                N_PE /= _K
            #else:
            #    _dw = 0

            qn_p = qn_w + qn_a + _mrg_bit
            if _type == 'avg':
                A_PEs = 0
            else:
                A_PEs = N_PE * syn_PE(d_PE, qn_w, qn_a, qn_p)
            #elif imp == 'anaRRAM':
            #    A_PEs = N_RRAM * d_RRAM['{}_{}_{}_a'.format(_C, _K, _dw)] * 1e12
            #elif imp == 'digRRAM':
            #    A_PEs = N_RRAM * d_RRAM['{}_{}_{}_{}b'.format(_C, _K, _dw, qn_w)] * 1e12
            fout1.write('{},{},{},{},'.format(_name, _type, N_PE, A_PEs))
            a_N[0], a_A[0] = N_PE, A_PEs

            # ----- Lr_sk, Wr_sk, Ar_sk ----- #
            '''
            Lr_sk = 0
            if _skip:
                if _I == 3 and _J == 3:
                    if _X == 32 and _Y == 32:
                        if _type == 'none':
                            Lr_sk = 37
                        elif _type == '2Y':
                            Lr_sk = 22
                        elif _type == '2XY':
                            Lr_sk = 22
                    elif _X == 16 and _Y == 16:
                        Lr_sk = 21
                    elif _X == 8 and _Y == 8:
                        Lr_sk = 13
                elif _I == 1 and _J == 1:
                    Lr_sk = 4
            if _type == '2Y' or _type == '2YS':
                Wr_sk = 2 * _C * qn_a
            elif _type == '2XY' or _type == '2XYS':
                Wr_sk = 4 * _C * qn_a
            else:
                Wr_sk = _C * qn_a
            Ar_sk = syn_fifo(d_fifo, 1, Lr_sk, Wr_sk)
            fout1.write('{},{},{},'.format(Lr_sk, Wr_sk, Ar_sk))
            a_N[1], a_A[1] = Lr_sk * Wr_sk, Ar_sk
            '''

            # ----- Nr_PEi, Lr_PEi, Wr_PEi, Ar_PEi, Nr_PEj, Wr_PEj, Ar_PEj, Wr_PEo, Ar_PEo ----- #
            if _type == 'none': #check
                Nr_PEi = _I - 1
                Lr_PEi = _J + _Y0 - 1
                Wr_PEi = qn_a * _C
                Nr_PEj = _I * (_J - 1)
                Wr_PEj = qn_a * _C
                Wr_PEo = qn_p * _K
            elif _type == 'pt_2Y':
                Nr_PEi = _I - 1
                Lr_PEi = np.floor((_J + _Y0) / 2)
                Wr_PEi = 2 * qn_a * _C
                Nr_PEj = _I * np.floor((_J - 1) / 2)
                Wr_PEj = 2 * qn_a * _C
                Wr_PEo = 2 * qn_p * _K
            elif _type == 'pt_2YS' or _type == 'pt_2Ym':
                Nr_PEi = _I - 1
                Lr_PEi = np.floor((_J + _Y0) / 2)
                Wr_PEi = 2 * qn_a * _C
                Nr_PEj = _I * np.floor((_J - 1) / 2)
                Wr_PEj = 2 * qn_a * _C
                Wr_PEo = qn_p * _K
            elif _type == 'pt_2XY':
                Nr_PEi = np.floor((_I - 1) / 2)
                Lr_PEi = np.floor((_J + _Y0) / 2)
                Wr_PEi = 4 * qn_a * _C
                Nr_PEj = _I * np.floor((_J - 1) / 2)
                Wr_PEj = 4 * qn_a * _C
                Wr_PEo = 4 * qn_p * _K
            elif _type == 'pt_2XYS' or _type == 'pt_2XYm':
                Nr_PEi = np.floor((_I - 1) / 2)
                Lr_PEi = np.floor((_J + _Y0) / 2)
                Wr_PEi = 4 * qn_a * _C
                Nr_PEj = _I * np.floor((_J - 1) / 2)
                Wr_PEj = 2 * qn_a * _C
                Wr_PEo = qn_p * _K
            else:
                Nr_PEi = 0
                Lr_PEi = 0
                Wr_PEi = 0
                Nr_PEj = 0
                Wr_PEj = 0
                Wr_PEo = 0
            Ar_PEi = syn_fifo(d_fifo, Nr_PEi, Lr_PEi, Wr_PEi)
            Ar_PEj = syn_fifo(d_fifo, Nr_PEj, 1, Wr_PEj)
            Ar_PEo = syn_fifo(d_fifo, 1, 1, Wr_PEo)
            fout1.write('{},{},{},{},{},{},{},{},{},'.format(Nr_PEi, Lr_PEi, Wr_PEi, Ar_PEi, Nr_PEj, Wr_PEj, Ar_PEj, Wr_PEo, Ar_PEo))
            a_N[1] = Nr_PEi * Lr_PEi * Wr_PEi + Nr_PEj * Wr_PEj + Wr_PEo
            a_A[1] = Ar_PEi + Ar_PEj + Ar_PEo

            # ----- Wr_padi, Ar_padi, Wr_pado, Ar_pado ----- #
            if _type == 'none':
                Wr_padi = qn_a * _C
                Wr_pado = qn_p * _K
            elif _type == 'pt_2Y':
                Wr_padi = 2 * qn_a * _C
                Wr_pado = 2 * qn_p * _K
            elif _type == 'pt_2YS' or _type == 'pt_2Ym':
                Wr_padi = 2 * qn_a * _C
                Wr_pado = qn_p * _K
            elif _type == 'pt_2XY':
                Wr_padi = 4 * qn_a * _C
                Wr_pado = 4 * qn_p * _K
            elif _type == 'pt_2XYS' or _type == 'pt_2XYm':
                Wr_padi = 4 * qn_a * _C
                Wr_pado = qn_p * _K
            else:
                Wr_padi = 0
                Wr_pado = 0
            Ar_padi = syn_fifo(d_fifo, 1, 1, Wr_padi)
            Ar_pado = syn_fifo(d_fifo, 1, 1, Wr_pado)
            fout1.write('{},{},{},{},'.format(Wr_padi, Ar_padi, Wr_pado, Ar_pado))
            a_N[2] = Wr_padi + Wr_pado
            a_A[2] = Ar_padi + Ar_pado

            # ----- N_act, A_act, N_SkMul, A_SkMul, N_avg, A_avg ----- #
            if _type == 'none':
                N_act = _K
                #N_SkMul = _C if _skip else 0
                N_avg = 0
            elif _type == 'pt_2Y':
                N_act = 2 * _K
                #N_SkMul = 2 * _C if _skip else 0
                N_avg = 0
            elif _type == 'pt_2YS' or _type == 'pt_2Ym':
                N_act = _K
                #N_SkMul = 2 * _C if _skip else 0
                N_avg = 0
            elif _type == 'pt_2XY':
                N_act = 4 * _K
                #N_SkMul = 4 * _C if _skip else 0
                N_avg = 0
            elif _type == 'pt_2XYS' or _type == 'pt_2XYm':
                N_act = _K
                #N_SkMul = 4 * _C if _skip else 0
                N_avg = 0
            else:
                N_act = 0
                #N_SkMul = 0
                N_avg = _C
            if _type == 'avg':
                A_act = 0
                #A_SkMul = 0
                A_avg = N_avg * syn_avg(d_avg, qn_a)
            else:
                A_act = N_act * syn_act(d_act, qn_b, qn_a)
                #A_SkMul = N_SkMul * d_skmul['{}'.format(qn_a)]
                A_avg = 0
            fout1.write('{},{},{},{},'.format(N_act, A_act, N_avg, A_avg))
            a_N[3], a_A[3] = N_act, A_act
            #a_N[5], a_A[5] = N_SkMul, A_SkMul
            a_N[4], a_A[4] = N_avg, A_avg

            # ----- R_PEs, Rr_syn, Rr_pad, R_act, R_avg ----- #
            a_R = a_A / np.sum(a_A) * 100
            fout1.write('{}%,{}%,{}%,{}%,{}%\n'.format(*a_R))

            a_N_acc += a_N
            a_A_acc += a_A
    with open('res_area.csv', 'a') as fout2:
        # ----- DNN, layer cfg, implementation, qn bit ----- #
        fout2.write('{},{},{}-bit,{}-bit,{}-bit,'.format(args.DNN_model, args.layer_cfg, qn_w, qn_b, qn_a))
        # ----- N_PEs, Br_syn, Br_pad, N_act, N_avg ----- #
        fout2.write('{},{},{},{},{},'.format(*a_N_acc))
        # ----- R_PEs, Rr_syn, Rr_pad, R_act, R_avg ----- #
        A_total = np.sum(a_A_acc)
        a_R_acc = a_A_acc / A_total * 100
        fout2.write('{}%,{}%,{}%,{}%,{}%,'.format(*a_R_acc))
        # ----- total area\ ----- #
        fout2.write('{}\n'.format(A_total))
