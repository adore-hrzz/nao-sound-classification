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

#include "features.h"
#include "config.h"


using namespace std;
using namespace cv;

string configPath = "SC.config";
ifstream configFileIN;

parameters par;
CvRTrees classificatorModel;
vector<string> classNames;

ifstream soundList;
ofstream modelData;

int windowSamples; //bytes

bool mapCompare(const pair<int,int> &i1, const pair<int,int> &i2){
    return i1.second < i2.second;
}

string getPrediction(string fileName){
    map<int, int> classResults;
    SF_INFO wavInfo;
    SNDFILE *wavFile = sf_open((par.soundFolder + fileName).c_str(), SFM_READ, &wavInfo);

    double *wavData=(double*) malloc(sizeof(double) * wavInfo.frames );
    long wavSamples = (double) sf_read_double(wavFile, wavData, wavInfo.frames);

    sf_close(wavFile);

    soundFeatures xTract;
    xTract.setData(wavData, wavSamples, par.subSamples , wavInfo.samplerate);

    for(int k = 0; k + par.windowSamples < wavSamples; k += par.windowSamples){
        vector <float> feat1;
        feat1.push_back(-1.0);

        map<char, vector<double> > segmentFeatures = xTract.calcFeatures(par.featParams, k, par.windowSamples);

        for (int k = 0; k < par.featParams.size(); k++)
            for(int i = 0; i < segmentFeatures[par.featParams[k]].size(); i++)
                feat1.push_back(segmentFeatures[par.featParams[k]][i]);


        Mat_<float> feat2 = Mat(1, feat1.size(), CV_32FC1, (float*)feat1.data());
        int subSamplePrediction = classificatorModel.predict(feat2, Mat());
        classResults[subSamplePrediction]++;

    }
    /*for (int k = 0; k < classNames.size(); k++){
        cout << classNames[k] << " -> " << classResults[k+1] << endl;
}*/

    int result = max_element(classResults.begin(),classResults.end(), mapCompare)->first-1;
    //cout << classNames[result]<< "(" << max_element(classResults.begin(),classResults.end(), mapCompare)->second << ")" << endl;
    if(!max_element(classResults.begin(),classResults.end(), mapCompare)->second) cout << wavInfo.frames << endl;
    return classNames[result];
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

    classificatorModel.load(((string)par.model).c_str());
    ifstream responseList;
    responseList.open(((string)par.classList).c_str());

    string fileName, fileClass;

    while (responseList >> fileClass) classNames.push_back(fileClass);

    modelData.open (par.featData, ofstream::out | ofstream::trunc);
    soundList.open (par.soundList, ifstream::in);
    map<pair<string, string>, int> results;
    while ((soundList >> fileName) && (soundList >> fileClass)){
        cout << fileName << endl;
        string prediction = getPrediction(fileName);
        results[pair<string, string>(fileClass, prediction)]++;
    }

// !!!!!!!!!!!!!!!!!!!
// HARDCODED FILE NAME
// !!!!!!!!!!!!!!!!!!!
    ofstream resultsFile("./results.csv");

    for( auto className2 : classNames)
        resultsFile << ";" << className2;
    resultsFile << endl;

    for(auto className1 : classNames){
        resultsFile << className1;
        for( auto className2 : classNames)
            resultsFile << ";" << results[pair<string, string>(className1, className2)];
        resultsFile << endl;
    }

    soundList.close();
    responseList.close();
    resultsFile.close();
    modelData.flush();
    modelData.close();

    return 0;
}
