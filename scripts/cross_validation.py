#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
import csv
import os
import sys
import itertools
from sklearn import svm, datasets
from sklearn.metrics import confusion_matrix


def plot_confusion_matrix(cm, classes, data_type, normalize=False, title='Confusion matrix', cmap=plt.cm.Blues):
    """This function prints and plots the confusion matrix. Normalization can be applied by setting 'normalize=True'."""
    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(classes))
    plt.xticks(tick_marks, classes, rotation=45)
    plt.yticks(tick_marks, classes)

    if normalize:
        cm = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
        cm = np.around(cm,2)
        print('Normalized confusion matrix for {} dataset'.format(data_type))
    else:
        print('Confusion matrix for {} dataset, without normalization'.format(data_type))

    print(cm)
    thresh = cm.max() / 2.
    for i, j in itertools.product(range(cm.shape[0]), range(cm.shape[1])):
        plt.text(j, i, cm[i, j], horizontalalignment="center", color="white" if cm[i, j] > thresh else "black")

    plt.tight_layout()
    plt.ylabel('True label')
    plt.xlabel('Predicted label')




def crossValidation(directory, csvfile):
    curAvg=np.zeros((5,5))
    n=0.0
    if len(sys.argv)==4:
        plt.figure()
        plt.ion()

    for i in range(len(os.listdir(directory))):
        csv.register_dialect('commadot', delimiter=';')
        f = open(csvfile+str(i+1)+'.csv', 'rb')
        reader = csv.reader(f, dialect='commadot')
        #generate confusion matrix of type numpy.array
        vocalization_labels=reader.next()[1::]
        matrix_list = []
        for rows in reader:
            matrix_list.append([ int(x) for x in rows[1::] ])
        matrix_list = np.asarray(matrix_list)
        f.close()
        #calculating moving average for every confusion matrix element
        curAvg = curAvg + (matrix_list - curAvg)/(n+1.0)
        n += 1
        if len(sys.argv)==4:
            plt.scatter(i,curAvg[sys.argv[2]][sys.argv[3]])

    np.set_printoptions(precision=2)
    plt.figure()
    plot_confusion_matrix(curAvg, classes=vocalization_labels, data_type=sys.argv[1], title='')
    plt.show()

    if len(sys.argv)==4:
        while True:
            plt.pause(0.05)


'''Main function'''
if __name__ == '__main__':
    if sys.argv[1]=='train':
        csvfile='./Resources/Train/results_train'
        directory='./Resources/Train'
    elif sys.argv[1]=='test':
        csvfile='./Resources/Test/results_test'
        directory='./Resources/Test'
    crossValidation(directory, csvfile)

