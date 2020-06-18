import numpy as np
import logging
import os
import load_CIFAR10 as ld
import util

#if __name__ == '__main__':
def main(args):
    if os.path.isdir(args.path_dataset_txt):
        return
    os.mkdir(args.path_dataset_txt)
    B = args.batch_size_txt # batch_size
    dataset = ld.fetch_data(args)
    logging.info('INFO: Dump image...')
    with open(args.path_dataset_txt + '/image.txt', 'w') as fout:
        if B >= 0 and B < dataset.test_images_len():
            a_image = np.transpose(np.reshape(dataset.test_images[:B], [-1, dataset.C, dataset.X0, dataset.Y0]), [0, 2, 3, 1])
        else:
            a_image = np.transpose(np.reshape(dataset.test_images, [-1, dataset.C, dataset.X0, dataset.Y0]), [0, 2, 3, 1])
        util.dump(fout, a_image, 'image')
        fout.write('image:mul\n0 0\n')
    logging.info('INFO: Dump label...')
    with open(args.path_dataset_txt + '/label.txt', 'w') as fout:
        if B >= 0 and B < dataset.test_images_len():
            a_label = dataset.test_labels[:B].astype(int)
        else:
            a_label = dataset.test_labels.astype(int)
        util.dump(fout, a_label, 'label')
        fout.write('label:mul\n0 0\n')
    logging.info('INFO: Dump complete!')
