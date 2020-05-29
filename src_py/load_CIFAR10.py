import pickle
import numpy as np

def fetch_data(args):
    file_tst = args.path_dataset + '/data_batch_'
    file_rst = args.path_dataset + '/test_batch'
    
    def unpickle(file_base):
        with open(file_base, 'rb') as fo:
            dictionary = pickle.load(fo, encoding='bytes')
        return dictionary
    
    datas_train = []
    labels_train = []
    for i in range(5):
        fetch = unpickle(file_tst + str(i+1))
        np_array = fetch[b'data']
        datas_train.append(np_array)
        array_o = fetch[b'labels']
        array = np.zeros([np.size(array_o),10], dtype=np.float32)
        for j in range(np.size(array_o)):
            array[j,array_o[j]] = 1
        labels_train.append(array)
    
    fetch = unpickle(file_rst)
    data_test = fetch[b'data']
    array_o = fetch[b'labels']
    label_test = np.zeros([np.size(array_o),10], dtype=np.float32)
    for j in range(np.size(array_o)):
        label_test[j,array_o[j]] = 1
    return CIFAR10(datas_train, labels_train, data_test, label_test)

class CIFAR10:
    def __init__(self, datas_train, labels_train, data_test, label_test):
        self.C = 3 # input channel 
        self.K = 10 # output channel
        self.X0= 32 # ifmap height
        self.Y0= 32 # ifmap width

        self.l_train_images = datas_train
        self.l_train_labels = labels_train
        self.test_images = data_test
        self.test_labels = label_test
        self.i_batch = 0
        self.i_set = 0
        self.i_test_batch = 0
    
    def train_images_len(self, index=None):
        n_len = 0
        if index == None:
            for train_images in self.l_train_images:
                n_len += train_images.shape[0]
        else:
            n_len = self.l_train_images[index].shape[0]
        return n_len
    
    def test_images_len(self):
        return len(self.test_images)
    
    def train_next_batch(self, n_batch):
        if self.i_batch + n_batch <= self.train_images_len(self.i_set):
            batch_train_images = self.l_train_images[self.i_set][self.i_batch: self.i_batch + n_batch]
            batch_train_labels = self.l_train_labels[self.i_set][self.i_batch: self.i_batch + n_batch]
            self.i_batch += n_batch
        elif self.i_set + 1 < len(self.l_train_images):
            batch_train_images = self.l_train_images[self.i_set + 1][0: n_batch]
            batch_train_labels = self.l_train_labels[self.i_set + 1][0: n_batch]
            self.i_batch = n_batch
            self.i_set += 1
        else:
            batch_train_images = self.l_train_images[0][0: n_batch]
            batch_train_labels = self.l_train_labels[0][0: n_batch]
            self.i_batch = n_batch
            self.i_set = 0
        return batch_train_images, batch_train_labels
    
    def test_next_batch(self, n_batch):
        if self.i_test_batch + n_batch <= self.test_images_len():
            batch_train_images = self.test_images[self.i_test_batch: self.i_test_batch + n_batch]
            batch_train_labels = self.test_labels[self.i_test_batch: self.i_test_batch + n_batch]
            #print(self.i_test_batch, 'to', self.i_test_batch + n_batch) # debug
            self.i_test_batch += n_batch
        else:
            batch_res = self.i_test_batch + n_batch - self.test_images_len()
            batch_train_images = np.concatenate((self.test_images[self.i_test_batch:], self.test_images[:batch_res]), axis=0)
            batch_train_labels = np.concatenate((self.test_labels[self.i_test_batch:], self.test_labels[:batch_res]), axis=0)
            #print(self.i_test_batch, 'to', self.test_images_len(), '; 0 to ', batch_res) # debug
            self.i_test_batch = batch_res
        
        return batch_train_images, batch_train_labels
    