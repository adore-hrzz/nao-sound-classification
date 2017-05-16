#!/usr/bin/env python

import sys
import random
import os


train_data_txt = 'train_data.txt'
test_data_txt = 'test_data.txt'

def splitData():
    #opening .txt file for reading
    f_data = open(sys.argv[1], 'r')
    name_list = f_data.read().splitlines()

    f_info = open(sys.argv[2], 'r')
    info_list = f_info.read().splitlines()

    rnd_list = dict()
    rnd_index = dict()
    data_index = dict()

    try:
        # Generate random index list for each class
        for k in xrange(0, len(info_list), 3):
            rnd_list[info_list[k]]= sorted(random.sample(xrange(0, int(info_list[k+1])), int(int(info_list[k+1])*0.25)))
            rnd_index[info_list[k]] = 0
            data_index[info_list[k]] = 0
        
        f_test = open(test_data_txt, 'w')
        f_train = open(train_data_txt, 'w')

        # For each file whose index is in random generated list, put it in Test, otherwise put it in Train
        for k in xrange(0, len(name_list), 2):
            name = name_list[k+1]
            if rnd_index[name] < len(rnd_list[name]):
                if data_index[name] == rnd_list[name][rnd_index[name]]:
                    f_test.write(name_list[k] + '\n'
                                +name_list[k+1] + '\n')
                    rnd_index[name] += 1
                else:
                    f_train.write(name_list[k] + '\n'
                                +name_list[k+1] + '\n')
            else:
                f_train.write(name_list[k] + '\n'
                                +name_list[k+1] + '\n')
            data_index[name] += 1
        f_test.close()
        f_train.close()
    finally:
        f_data.close()
        f_info.close()

'''Main function'''
if __name__ == '__main__':
    splitData()
