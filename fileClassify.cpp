#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cmath>

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

CvRTrees classModel;
map<int, string> classNames;
SF_INFO wavInfo;

bool mapCompare(const pair<int,int> &i1, const pair<int,int> &i2){
    return i1.second < i2.second;
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

int classifySegment(double* segmentData, int segmentLength, string featString){
    map<int, int> classResults;

    soundFeatures xTract;
    xTract.setData(segmentData, segmentLength, par.subSamples , wavInfo.samplerate);

    for(int k = 0; k + par.windowSamples < segmentLength; k += par.windowSamples){
        vector <float> feat1;
        feat1.push_back(-1.0);

        map<char, vector<double> > segmentFeatures = xTract.calcFeatures(featString, k, par.windowSamples);

        for (int k = 0; k < featString.size(); k++)
            for(int i = 0; i < segmentFeatures[featString[k]].size(); i++)
                feat1.push_back(segmentFeatures[featString[k]][i]);


        Mat_<float> feat2 = Mat(1, feat1.size(), CV_32FC1, (float*)feat1.data());

        int subSamplePrediction = classModel.predict(feat2, Mat());
        classResults[subSamplePrediction]++;
    }
    for (int k = 1; k <= classNames.size(); k++)
            cout << classNames[k] << " -> " << classResults[k] << endl;
    int result = max_element(classResults.begin(),classResults.end(), mapCompare)->first;
 //   cout << classNames[result]<< "(" << max_element(classResults.begin(),classResults.end(), mapCompare)->second << ")" << endl;
    return result;
}


int main(int argc, char *argv[]){
    readParams(argc, argv);

    classModel.load(((string)par.model).c_str());

    ifstream respFile;
    respFile.open(((string)par.classList).c_str());

    string temp;

    for(int k = 0;respFile >> temp; k++)
        classNames[k+1]=temp;
    respFile.close();

    SNDFILE *wavFile = sf_open("/home/mirko/nao/workspace/SoundClass/Resources/test3.wav", SFM_READ, &wavInfo);

    double *wavData=(double*) malloc(sizeof(double) * wavInfo.frames );
    long long wavSamples = (double) sf_read_double(wavFile, wavData, wavInfo.frames);

    sf_close(wavFile);

    int noInput = 3;
    long long lastSampleStart = 0;

    for (long long k = 0; k + wavInfo.samplerate/4 < wavSamples; k+=wavInfo.samplerate/4){
        if (noInput > 1){
            if(noInput == 2){
                long double startSec = (double)lastSampleStart / wavInfo.samplerate;
                long long startMin = startSec / 60;
                startSec -= startMin*60;
                long double endSec = (double)k / wavInfo.samplerate;
                long long endMin = endSec / 60;
                endSec -= endMin*60;

                cout << "[" << startMin << ":" << startSec << " - "<< endMin << ":" << endSec << "] " << classNames[classifySegment(wavData+lastSampleStart, (k - lastSampleStart - wavInfo.samplerate/2), par.featParams)] << endl;
            }
            lastSampleStart = k;
        }
        bool flagClassify = false;
        double temp2=0;

        for(int i=0;i < wavInfo.samplerate/4;i++){
            if (temp2 < wavData[k+i])temp2=wavData[k+i];
            if (wavData[k+i] > 0.02){
                flagClassify = true;
                if(noInput){
                    noInput = 0;
                    lastSampleStart = k+i;
                }
                break;
            }
        }
       //  cout << temp2 << endl;
        if(!flagClassify){
            noInput++;
            continue;
        }


    }

    return 0;
}
