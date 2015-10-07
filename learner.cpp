/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>

#include <sndfile.h>

#include "noiseFilter.h"
#include "features.h"
#include "config.h"


using namespace std;
using namespace cv;

string configPath = "/home/mirko/nao/workspace/SoundClass/Resources/SC.config";
ifstream configFileIN;

parameters par;
string fsToTest;
string fsSelected = "";
map<char, pair<int, int> > fsSize;
int fsNum = 1;

ifstream soundList;
ofstream modelData;

noiseFilter filter;

int windowSamples; //bytes

int randomGen (int n) {return rand() % n;}

void getFeatures(string soundName, string featString){
    string soundResponse = soundName.substr(soundName.find('_')+1);

    SF_INFO wavInfo;
    SNDFILE *wavFile = sf_open((par.soundFolder + soundName + ".wav").c_str(), SFM_READ, &wavInfo);

    double *wavData=(double*) malloc(sizeof(double) * wavInfo.frames );
    long wavSamples = (double) sf_read_double(wavFile, wavData, wavInfo.frames);

    sf_close(wavFile);

    soundFeatures xTract;
    xTract.setData(wavData, wavSamples, par.subSamples , wavInfo.samplerate);

    for(int k = 0; k + par.windowSamples < wavSamples; k += par.windowSamples){
        map<char, vector<double> > segmentFeatures = xTract.calcFeatures(featString, k, par.windowSamples);

        modelData << soundResponse;

        for (int i = 0; i < featString.size(); i++){
            if (k == 0 && soundName.substr(0,2) == "1_"){
                fsSize[featString[i]].first = fsNum;
                fsNum += segmentFeatures[featString[i]].size();
                fsSize[featString[i]].second = fsNum;
            }
            for(int j = 0; j < segmentFeatures[featString[i]].size(); j++){
                if(isfinite(segmentFeatures[featString[i]][j])){
                    if (segmentFeatures[featString[i]][j] < pow(10,-14)) segmentFeatures[featString[i]][j] = 0;
                    if (segmentFeatures[featString[i]][j] > pow(10,6)) segmentFeatures[featString[i]][j] = pow(10,6);
                    modelData << ", "<< segmentFeatures[featString[i]][j];
                }
                else modelData << ", @";
            }
        }
        modelData << endl;
    }

    cout << "DONE: " << soundName << endl;
}

void FSclassModel(){

    CvMLData modelData;
    modelData.set_miss_ch('@');
    modelData.read_csv((par.featData).c_str());
    modelData.set_response_idx(0);
    CvTrainTestSplit test((float)0.7, true);
    modelData.set_train_test_split(&test);

    if(fsSize.begin()->first == '\0') fsSize.erase(fsSize.begin());

    for(map<char, pair<int,int> >::iterator it = fsSize.begin() ; it != fsSize.end() ; ++it)
        for(int i = it->second.first; i < it->second.second; i++)
            modelData.chahge_var_idx(i, false);

    fsToTest = par.featParams;

    double errPercent = 100;
    while(!fsToTest.empty()){
        double errMin = 100;
        int errCh;

        for(int k = 0; k < fsToTest.size(); k++){
            for(int i = fsSize[fsToTest[k]].first; i < fsSize[fsToTest[k]].second; i++)
                        modelData.chahge_var_idx(i, true);
            double errTemp = 0;
            for(int avgCount = 0; avgCount < 10; avgCount++){
                int randNum = rand() % 5;
                for(int k = 0; k < randNum; k++) modelData.mix_train_and_test_idx();

                CvGBTreesParams params;
                params.loss_function_type = CvGBTrees::DEVIANCE_LOSS;
                CvGBTrees classModel;

                classModel.train(&modelData, params, false);

                errTemp += classModel.calc_error(&modelData, CV_TEST_ERROR,  0);
            }
            errTemp /= 10;
            cout << "(" + fsSelected + fsToTest[k] + " -> " << errTemp << ") ";
            cout.flush();
            if (errMin > errTemp){
                errMin = errTemp;
                errCh = k;
            }
            for(int i = fsSize[fsToTest[k]].first; i < fsSize[fsToTest[k]].second; i++)
                        modelData.chahge_var_idx(i, false);
        }

        cout << endl;

        if(errMin >= errPercent){
            cout << "DONE: " << fsSelected << endl;
            break;
        }

        for(int i = fsSize[fsToTest[errCh]].first; i < fsSize[fsToTest[errCh]].second; i++)
                    modelData.chahge_var_idx(i, true);

        errPercent = errMin;
        fsSelected += fsToTest[errCh];
        sort(fsSelected.begin(), fsSelected.end());
        fsToTest.erase(fsToTest.begin() + errCh);

        cout << "SELECTED: " << fsSelected + " -> " << errPercent << "% Error" << endl;
    }

}

void ENDclassModel(string featString){
    CvMLData modelData;
    modelData.set_miss_ch('@');
    modelData.read_csv((par.featData).c_str());

    modelData.set_response_idx(0);

    map<string, int> fsClasses = modelData.get_class_labels_map();

    CvGBTreesParams params;
    params.loss_function_type = CvGBTrees::DEVIANCE_LOSS;
    CvGBTrees classModel;

    classModel.train(&modelData, params, false);
    classModel.save((par.model).c_str());

    ofstream classesFile;
    classesFile.open(par.classList, ofstream::out | ofstream::trunc);
    for(int k=0; k < fsClasses.size(); k++)
        for(map<string, int>::iterator it = fsClasses.begin(); it != fsClasses.end(); ++it)
            if(it->second == k+1)classesFile << it->first << endl;

    remove("temporary.csv");
    classesFile.flush();
    classesFile.close();
}


void readParams(int argc, char *argv[]){
    if (argc > 1) configPath = argv[1];

    string temp;
    string params[10];

    configFileIN.open(configPath);
    for (int k=0; configFileIN >> temp; k++) params[k] = temp;
    configFileIN.close();

    par.fill(params);
}

int main(int argc, char *argv[]){
    readParams(argc, argv);
    srand(time(NULL));

    string soundName;

    cout << "Select one of the options:\n 1:\tGenerate machine learning model\n 2:\tRun feature selection algorithm\n 3:\tBoth actions\n";
    string c;
    while(c != "1" && c != "2" && c != "3") cin >> c;

    fsSize.clear();

    modelData.open (par.featData, ofstream::out | ofstream::trunc);
    soundList.open (par.soundList, ifstream::in);
    while (soundList >> soundName)
        getFeatures(soundName, par.featParams);
    soundList.close();
    modelData.flush();
    modelData.close();


    fsToTest = par.featParams;

    if(c == "1") ENDclassModel(par.featParams);

    if(c == "2") FSclassModel();

    if(c == "3") {
        FSclassModel();

        modelData.open (par.featData, ofstream::out | ofstream::trunc);
        soundList.open (par.soundList, ifstream::in);
        while (soundList >> soundName) getFeatures(soundName, fsSelected);
        soundList.close();
        modelData.flush();
        modelData.close();
        ENDclassModel(fsSelected);
    }

    return 0;
}
