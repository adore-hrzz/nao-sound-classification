#include "SCModule.h"
#include <unistd.h>

#define arrLen(x) (sizeof(x)/sizeof(x[0]))

bool mapCompare(const pair<int,int> &i1, const pair<int,int> &i2){
    return i1.second < i2.second;
}

SCModule::SCModule(boost::shared_ptr<ALBroker> pBroker, std::string pName) : ALSoundExtractor(pBroker, pName), fProxyToALMemory(getParentBroker())
{
  setModuleDescription("Sound classification modul with option send recorded sound");

  functionName("soundAcquisition", getName(), "Get raw sound data from microphone");
  addParam("duration", "Duration of recordingg in seconds");
  setReturn("AL::ALValue","Raw sound data");
  BIND_METHOD(SCModule::soundAcqusition);

  functionName("startRTClassification", getName(), "Start listening for sound and do automatic sound classification");
  BIND_METHOD(SCModule::startRTClassification);

  functionName("stopRTClassification", getName(), "Stop live sound classification");
  BIND_METHOD(SCModule::stopRTClassification);

  functionName("initRTClassification", getName(), "Initialize live sound classification");
  addParam("modelPath", "Path to saved machine learning model");
  addParam("featString", "String of features to calculate");
  addParam("responses", "Path to file containing response strings");
  BIND_METHOD(SCModule::initRTClassification);

  functionName("soundClassification", getName(), "Classification triggered on microEvent");
  addParam("key", "Not used");
  addParam("value", "Index of recorded sound in sound buffer");
  addParam("message", "Not used");
  BIND_METHOD(SCModule::soundClassification);
}

SCModule::~SCModule()
{
    stopDetection();
    audioDevice->destroyConnection();
    model.clear();
    fProxyToALMemory.unsubscribeToMicroEvent("classifyAudio", "SCModule");
}
void SCModule::init()
{
    fProxyToALMemory.declareEvent("soundClassified", "SCModule");
    fProxyToALMemory.subscribeToMicroEvent("classifyAudio", "SCModule", "dummy", "soundClassification");
    flagRecord = false;
    flagRealTime = false;
    flagClassify = false;
    flagReady = false;

    recordedNoise.reserve(36000);

    noInput = 0;
    treshold = 0.6;
    windowSamples = pow (2, 11);
    subWindow = pow (2, 9);
    samplerate = 16000;
}


ALValue SCModule::soundAcqusition( const ALValue &duration ){
    audioDevice->callVoid("setClientPreferences",
        getName(),
        samplerate,         //frekvencija
        (int)FRONTCHANNEL,  //channel
        0                   //deinterleaving
        );

    noiseFilter filter;

    recordedNoise.clear();

    startDetection();
    cout << "Acquiring noise profile." << endl;

    for (int k = 3; k>0; k--){
        cout << k << endl;
        sleep(1);
    }

    cout << "Initializing noise filter." << endl;

    filter.initFilter(recordedNoise.size(), &recordedNoise[0]);

    recordedAudio.clear();
    recordedAudio.push_back(vector<double>());

    cout << "RECORD!" << endl;

    while(recordedAudio[0].size() < (float)duration * samplerate) flagRecord = true;

    stopDetection();
    flagRecord = false;

    cout << "END" << endl << "Reducing noise in recording." << endl;

//    filter.reduceNoise(recordedAudio[0].size(), &recordedAudio[0][0]);

    cout << "Done! Noise is reduced." << endl;

    vector <float> temp (recordedAudio[0].begin(), recordedAudio[0].end());
    return ALValue(temp);
}

void SCModule::process(const int & nbOfChannels,
               const int & nbrOfSamplesByChannel,
               const AL_SOUND_FORMAT * buffer,
               const ALValue & timeStamp)
{
//    std::cout << "In process" << std::endl;
    if(flagRecord){
        bool flagInput = false;
        for(int k=0; k<nbrOfSamplesByChannel ; k++)
            if ((double) buffer[k]/32767 > treshold)
                flagInput=true;

        if(flagInput){
            recordedAudio[0].reserve(recordedAudio[0].size() + nbrOfSamplesByChannel);
            for(int k=0; k<nbrOfSamplesByChannel ; k++)
                recordedAudio[0].push_back((double)buffer[k]/32767);
        }
    }
    else if(flagRealTime){
//        std::cout << "in flag real time" << std::endl;
        if (!flagClassify && numDone == numAudio && numDone > 0) {
            numAudio = 0;
            numDone = 0;
            vector<vector<double> >().swap(recordedAudio);
        }
        if(noInput > 5 && flagClassify){
            flagClassify = false;
            fProxyToALMemory.raiseMicroEvent("classifyAudio", ALValue(numAudio++));
            vector<double> temp;
            recordedAudio.push_back(temp);
        }
        bool flagInput = false;
        for(int k=0; k<nbrOfSamplesByChannel ; k++)
            if ((double) buffer[k]/32767 > treshold) flagInput=true;
        if(flagInput){
            flagClassify = true;
            noInput = 0;
            recordedAudio[numAudio].reserve(recordedAudio[numAudio].size() + nbrOfSamplesByChannel);
            for(int k=0; k<nbrOfSamplesByChannel ; k++)
                recordedAudio[numAudio].push_back((double)buffer[k]/32767);
        }
        else{
            noInput++;
            for(int k = 0; k < nbrOfSamplesByChannel; k++)
                recordedNoise.push_back((double)buffer[k]/32767);
            if(recordedNoise.size() >= 32768)
                recordedNoise.erase(recordedNoise.begin(), recordedNoise.begin() + (recordedNoise.size()- 32767));
        }
    }
    else{
        for(int k = 0; k < nbrOfSamplesByChannel; k++)
            recordedNoise.push_back((double)buffer[k]/32767);
        if(recordedNoise.size() >= 32768)
            recordedNoise.erase(recordedNoise.begin(), recordedNoise.begin() + (recordedNoise.size()- 32767));
    }
}

void SCModule::startRTClassification(){
    std::cout << "Starting RT classification" << std::endl;
    if(!flagReady) {
        std:: cout << "Not ready" << std::endl;
        return;
    }
    recordedAudio.clear();
    numAudio = 0;
    recordedAudio.push_back(vector <double>());
    numDone = 0;

    flagRealTime = true;

    audioDevice->callVoid("setClientPreferences",
        getName(),
        samplerate,         //frekvencija
        (int)FRONTCHANNEL,  //channel
        0                   //deinterleaving
        );

    startDetection();
}
void SCModule::stopRTClassification(){
    std::cout << "Stopping RT classification" << std::endl;
    flagRealTime = false;
    stopDetection();
}

void SCModule::soundClassification(const std::string &key,
                                    const AL::ALValue &value,
                                    const AL::ALValue &message){
    noiseFilter filter;
    filter.initFilter(recordedNoise.size(), &recordedNoise[0]);
    filter.reduceNoise(recordedAudio[(int)value].size(), &recordedAudio[(int)value][0]);

    soundFeatures xTract;
    xTract.setData(&recordedAudio[(int)value][0], recordedAudio[(int)value].size(), subWindow, samplerate);

    classResults.clear();

    for(int k = 0; k + windowSamples < recordedAudio[(int)value].size(); k += windowSamples){
        vector <float> feat1;
        feat1.push_back(-1.0);

        map<char, vector<double> > segmentFeatures = xTract.calcFeatures(classParams, k, windowSamples);

        for (int i = 0; i < classParams.size(); i++)
            for(int j = 0; j < segmentFeatures[classParams[i]].size(); j++)
                feat1.push_back(segmentFeatures[classParams[i]][j]);


        Mat_<float> feat2 = Mat(1, feat1.size(), CV_32FC1, (float*)feat1.data());

        int asdad = model.predict(feat2, Mat());

        cout << asdad << " ";
        classResults[asdad]++;
    }

    int result = max_element(classResults.begin(),classResults.end(), mapCompare)->first;
    cout << classNames[result]<< "(" << max_element(classResults.begin(),classResults.end(), mapCompare)->second << ")" << endl;
    fProxyToALMemory.raiseEvent("soundClassified", classNames[result]);
}

void SCModule::initRTClassification(const AL::ALValue &modelPath, const ALValue &featString, const AL::ALValue &responses){
    classParams = (string)featString;

    model.load(((string)modelPath).c_str());

    ifstream respFile;
    respFile.open(((string)responses).c_str());

    string temp;

    for(int k = 0;respFile >> temp; k++)
        classNames[k+1]=temp;
    respFile.close();

    flagReady = true;
}
