# SA_sim
Code accompanying the paper
> Design and Analysis of Systolic Array-Based Accelerators for Convolutional Neural Networks \
> Yen-Ting Lin, Cheng-Wen Wu.

## Requirements
```
Python == 3.7.7, TensorFlow == 1.15.0, SystemC-2.3.3, Design Compiler Version Q-2019.12
```

## Training, Performance Evaluation, and Result Validation
```
python3 src_py/run.py
```

### Define Neural Network
In `src_py/CNN_ctor.py`, define functions to construct the neuro network with different layer configurations.
```py
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
```
_para. 1_: `'p1'`;
_para. 2_: `'p2'`;
_para. 4_: `'p4'`.

### Results
The simulated result is reported to `res_<model name>_<config name>_<PE array option>_<quantization option>.csv`.

It summarize the **total cycle**, **wait cycle**, **valid cal cycle**, **invalid cal cycle**, **_U_cal_**, and **_U_total_** of each layer.

The **classification accuracy** means accuracy of SystemC simulator's result comparing to the batch of test set which is set in `src_py/run.py`.

The **reference accuracy** means accuracy of SystemC simulator's result comparing to the TensorFlow inference's result.

## Area Evaluation
If you complete the simulation of SystemC and TensorFlow, please move `cfg_dt.csv` into `Area_eval/` for area evaluation.
```
mv cfg_dt.csv Area_eval/
cd Area_eval/
python3 run.py
```

### Results
5

6
