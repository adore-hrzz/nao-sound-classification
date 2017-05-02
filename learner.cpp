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
#include <cmath>
#include <unistd.h>

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
map<char, pair<int, int> > fsSize;
map<char, vector<int> > fsNormalization;
bool fsNormSet = false;
bool fsClassSet = false;
int fsNum = 0;

ifstream soundList;
ofstream modelData;

noiseFilter filter;

int windowSamples; //bytes

void getFeatures(string soundName, string featString){
    string soundResponse = soundName.substr(soundName.find("__")+2);
    soundResponse.erase(soundResponse.find("_"));

    SF_INFO wavInfo;
    SNDFILE *wavFile = sf_open((par.soundFolder + soundName).c_str(), SFM_READ, &wavInfo);

    double *wavData=(double*) malloc(sizeof(double) * wavInfo.frames );
    long wavSamples = (double) sf_read_double(wavFile, wavData, wavInfo.frames);

    sf_close(wavFile);

    soundFeatures xTract;
    xTract.setData(wavData, wavSamples, par.subSamples , wavInfo.samplerate);

    for(int k = 0; k + par.windowSamples < wavSamples; k += par.windowSamples){

        map<char, vector<double> > segmentFeatures = xTract.calcFeatures(featString, k, par.windowSamples);

        modelData << soundResponse;

        for (int i = 0; i < featString.size(); i++){
            if (!fsClassSet){
                fsSize[featString[i]].first = fsNum;
                fsNum += segmentFeatures[featString[i]].size();
                fsSize[featString[i]].second = fsNum;
            }
            for(int j = 0; j < segmentFeatures[featString[i]].size(); j++){
                if(!fsNormSet)fsNormalization[featString[i]].push_back(log10(abs(segmentFeatures[featString[i]][j])));
                if(isfinite(segmentFeatures[featString[i]][j])){//*pow((double)10,-fsNormalization[featString[i]][j])){
                    modelData << ", "<< (double) segmentFeatures[featString[i]][j];//*pow((double)10,-fsNormalization[featString[i]][j]);
                }
                else modelData << ", @";
            }
        }
        modelData << endl;
        fsNormSet = true;
        fsClassSet = true;
        int prog = (double)(k+ par.windowSamples)/wavSamples*100;
        cout << soundName << ": [" << string(prog/10,'|') << string(10 - prog/10,'*')<< "] "<< prog << "%\r" << flush;
    }
    cout << soundName << ": [" << string(10,'|') << "] DONE" << endl;
}

void classModel(string featString){
    CvMLData modelData;
    modelData.set_miss_ch('@');
    modelData.read_csv((par.featData).c_str());

    modelData.set_response_idx(0);

    map<string, int> fsClasses = modelData.get_class_labels_map();

    CvRTParams params;
    params.calc_var_importance = true;
    params.max_depth = 10;
    params.min_sample_count = 5;
    params.max_categories = 5;

    CvRTrees classModel;

    classModel.train(&modelData, params);
    classModel.save((par.model).c_str());

   // cout << classModel.get_train_error() << endl;

    Mat varImp = classModel.getVarImportance();
    for (int i = 0; i < featString.size(); i++){
        double tempImp=0;
        for (int k = fsSize[featString[i]].first; k < fsSize[featString[i]].second; k++)
            tempImp+=varImp.at<float>(k);
        if(tempImp) cout << fsSize[featString[i]].second - fsSize[featString[i]].first << " " << featString[i] << " -> " << tempImp << endl;
    }

    ofstream classesFile;
    classesFile.open(par.classList, ofstream::out | ofstream::trunc);
    for(int k=0; k < fsClasses.size(); k++)
        for(map<string, int>::iterator it = fsClasses.begin(); it != fsClasses.end(); ++it)
            if(it->second == k+1)classesFile << it->first << endl;


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

    string soundName;

    fsSize.clear();


    modelData.open (par.featData, ofstream::out | ofstream::trunc);
    soundList.open (par.soundList, ifstream::in);
    while (soundList >> soundName)
        getFeatures(soundName, par.featParams);
    soundList.close();
    modelData.flush();
    modelData.close();

    classModel(par.featParams);


    return 0;
}
