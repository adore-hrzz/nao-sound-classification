#!/bin/bash

for i in {1..100} 
do
    split_train_test_data.py ./Resources/output.txt ./Resources/class_info.txt
    ./Learner SC_train.config
    ./folderClassify SC_train.config
    mv results.csv ./Resources/Train/results_train$i.csv
    ./folderClassify SC_test.config
    mv results.csv ./Resources/Test/results_test$i.csv
done
