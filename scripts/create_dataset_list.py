#!/usr/bin/env python
# -*- coding: utf-8 -*-
 
import os
import sys
import csv

csvName='output.csv'
txtName='output.txt'


def createList():
    """Function takes directory path as an argument, goes through all subdirectories in it and creates CSV file and txt file containing relative paths of all .wav files in subdirectories and class name for each sample. If no argument is passed, it takes the current directory as the argument."""
    if len(sys.argv) == 1:
        rootDir = '.'
        print('WARNING: No directory argument provided! Traversing current directory recursively!')
    else:
        rootDir = sys.argv[1]
    
    #opening CSV file for writing
    csv.register_dialect('commadot', delimiter=';')
    f = open(csvName, 'wt')
    writer = csv.writer(f, dialect='commadot')
    #opening txt file for writing
    txtfile = open(txtName, 'w')

    try:      
        for dir_, _, files in os.walk(rootDir):
            for fileName in files:
                if fileName.lower().endswith(".wav"):
                    try:
                        relDir = os.path.relpath(dir_, rootDir)
                        relFile = os.path.join(relDir, fileName)

                        s=fileName
                        s1 = s.split('__')[1]
                        s2 = s1.split('_')[0]

                        if(relDir=='.'):
                            writer.writerow( (fileName, s2) )
                            txtfile.write(fileName + '\n')
                            txtfile.write(s2 + '\n')
                        else:
                            writer.writerow( (relFile, s2) )
                            txtfile.write(fileName + '\n')
                            txtfile.write(s2 + '\n')
                    except:
                        pass          
    finally:
        f.close()
        txtfile.close()



'''Main function'''
if __name__ == '__main__':
    createList()
