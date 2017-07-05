#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import csv
import itertools
import numpy as np
import matplotlib.pyplot as plt
from sklearn import svm, datasets
from sklearn.model_selection import train_test_split
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


def validator(csvfile):
    csv.register_dialect('commadot', delimiter=';')

    #opening and reading CSV file containing Train dataset
    print(csvfile)
    f_train = open(csvfile, 'rb')
    reader = csv.reader(f_train, dialect='commadot')

    #generate vocalization labels in order and confusion matrix for Train dataset
    vocalization_labels=reader.next()[1::]    
    matrix_list=[]
    for rows in reader:
        matrix_list.append([ int(x) for x in rows[1::] ])
    cnf_matrix = np.asarray(matrix_list)
    f_train.close()

    #plot confusion matrix for Train dataset
    np.set_printoptions(precision=2)
    plt.figure()
    plot_confusion_matrix(cnf_matrix, classes=vocalization_labels, data_type=csvfile, title='')
    plt.show()


'''Main function'''
if __name__ == '__main__':

    csvfile = sys.argv[1]
    validator(csvfile)

