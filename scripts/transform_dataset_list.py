#!/usr/bin/env python
# -*- coding: utf-8 -*-

import csv
import sys
import numpy
import os
import soundfile as sf


transformed_txt = 'transformed_output.txt'
information_txt = 'class_info.txt'


def transformList():
    '''Function takes three arguments from command line: .txt file path, old class name, new class name. If only the first argument is assigned when calling a function, it will calculate how many audio files of which class exist and what is their total duration. If all three arguments are assigned, it will create a new .txt file with the old class name replaced by new class name.'''

    #opening .txt file for reading
    f_data = open(sys.argv[1], 'r')
    name_list = f_data.read().splitlines()

    f_info = open(information_txt, 'w')

    rootDir = sys.argv[2]

    try:    
        if len(sys.argv) == 3:
            #calculating durations and members of each sample class
            dataset={}
            i=0
            while i<len(name_list):
                fullPath = os.path.join(rootDir, name_list[i])
                sound = sf.SoundFile(fullPath)
                if dataset.has_key(name_list[i+1]):
                    dataset[name_list[i+1]]['duration'] += len(sound)/float(sound.samplerate)
                    dataset[name_list[i+1]]['count'] += 1
                else:                    
                    dataset[name_list[i+1]] = {'duration':len(sound)/float(sound.samplerate), 'count':1}
                i += 2
            
            for k,v in dataset.items():
                print ('{}: {} samples with total duration: {:.2f} seconds').format(k,v['count'],v['duration'])
                f_info.write(str(k)+'\n')
                f_info.write(str(v['count'])+'\n')
                f_info.write(str(v['duration'])+'\n')                

        elif len(sys.argv) == 5:
            #changing class name and writing it to a new CSV file
            f2 = open(transformed_txt, 'w')
            i=0
            while i<len(temp):
                if temp[i+1] == sys.argv[3]:
                    f2.write(name_list[i]+'\n')
                    f2.write(sys.argv[4]+'\n')
                else:
                    f2.write(name_list[i]+'\n')
                    f2.write(name_list[i+1]+'\n')
                i += 2
            f2.close()                  
    finally:
        f_data.close()
        f_info.close()


'''Main function'''
if __name__ == '__main__':
    transformList()
