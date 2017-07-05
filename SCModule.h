#ifndef SCMODULE_H
#define SCMODULE_H

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#include <unistd.h>
#include <time.h>

#include <boost/shared_ptr.hpp>

#include <alcommon/almodule.h>
#include <alcommon/alproxy.h>
#include <alcommon/albroker.h>
#include <alvalue/alvalue.h>
#include <alaudio/alsoundextractor.h>
#include <alproxies/almemoryproxy.h>

#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>

#include "features.h"
#include "noiseFilter.h"

using namespace std;
using namespace AL;
using namespace cv;

class SCModule : public ALSoundExtractor
{
private:
    bool flagRecord;
    bool flagRealTime;
    bool flagClassify;
    bool flagNoise;
    bool flagReady;

    int noInput;
    float treshold;
    int windowSamples;
    int subWindow;
    int samplerate;

    string classParams;
    map<int,int> classResults;
    map<int, string> classNames;
    int numDone;

    CvRTrees model;

    vector<vector <double>> recordedAudio;
    vector<double> recordedNoise;
    int numAudio;

    ALMemoryProxy fProxyToALMemory;

public:
    SCModule(boost::shared_ptr<ALBroker> pBroker, std::string pName);
    virtual ~SCModule();
    virtual void init();

    ALValue soundAcqusition(const ALValue&duration);

    void process(const int & nbOfChannels,
                   const int & nbrOfSamplesByChannel,
                   const AL_SOUND_FORMAT * buffer,
                   const ALValue & timeStamp);

    void initRTClassification(const AL::ALValue &modelPath, const AL::ALValue &featString, const AL::ALValue &responses);
    void startRTClassification();
    void stopRTClassification();

    void soundClassification(const std::string &key,
                             const AL::ALValue &value,
                             const AL::ALValue &message);

};

#endif // SCMODULE_H
