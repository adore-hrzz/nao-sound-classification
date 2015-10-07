/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <iostream>
#include <vector>
#include <fstream>

#include <alerror/alerror.h>
#include <alcommon/alproxy.h>
#include <alvalue/alvalue.h>

#include <sndfile.h>
#include "config.h"
#include "noiseFilter.h"

using namespace std;
using namespace AL;

string configPath = "/home/mirko/nao/workspace/SoundClass/Resources/SC.config";
ifstream configFileIN;

ifstream soundListIN;
ofstream soundListOUT;

parameters par;

int lastNUM;

void readParams(int argc, char *argv[]){
    if (argc > 1) configPath = argv[1];

    string temp;
    string params[10];

    configFileIN.open(configPath, ifstream::in);
    for (int k=0; configFileIN >> temp; k++)
        params[k] = temp;
    configFileIN.close();

    par.fill(params);

}

int main(int argc, char *argv[]){

    readParams(argc, argv);

    soundListIN.open (par.soundList, ifstream::in);
    soundListIN.seekg (-1, ios_base::end);
    char ch = 'm';
    while(ch != '\n'){
                soundListIN.seekg(-2, ios_base::cur);

                if((int)soundListIN.tellg() <= 0){
                    soundListIN.seekg(0);
                    break;
                }
                soundListIN.get(ch);
            }
    if(soundListIN.tellg() == -1){
        lastNUM = 0;
    }
    else{
        string temp;
        getline (soundListIN, temp);
        lastNUM = stoi(temp.substr(0,temp.find('_')));
    }
    soundListIN.close();
    soundListOUT.open(par.soundList, ofstream::out | std::ofstream::app);
    try {
        bool rec = true;
        boost::shared_ptr<AL::ALProxy> testProxy = boost::shared_ptr<AL::ALProxy>(new AL::ALProxy("SCModule", par.robotIP, par.robotPort));
        while(rec){
/*            {
               SF_INFO wavInfo;
                SNDFILE *wavFile = sf_open(("/home/mirko/nao/workspace/SoundClass/Resources/0_noise.wav"), SFM_READ, &wavInfo);

                double *wavData=(double*) malloc(sizeof(double) * wavInfo.frames );
                long wavSamples = (double) sf_read_double(wavFile,wavData,wavInfo.frames);

                noiseFilter proba;
                proba.initFilter(wavSamples, wavData);

                wavFile = sf_open(("/home/mirko/nao/workspace/SoundClass/Resources/1_govor.wav"), SFM_READ, &wavInfo);
                wavData=(double*) malloc(sizeof(double) * wavInfo.frames );
                wavSamples = (double) sf_read_double(wavFile,wavData,wavInfo.frames);

                proba.reduceNoise(wavSamples, wavData);

                wavFile = sf_open(("/home/mirko/nao/workspace/SoundClass/Resources/cist.wav"), SFM_WRITE, &wavInfo);
                sf_writef_double(wavFile, wavData, wavSamples);
                sf_write_sync(wavFile);
                sf_close(wavFile);

                break;
            }
*/
            float duration;
            cout << "Please enter sound recording duration in seconds (without silent parts):" << endl;
            cin >> duration;
            cout << "First, three seconds will be recorded to sample surrounding noise" << endl;
            ALValue zvukAL = testProxy->call<ALValue>("soundAcquisition", ALValue(duration));
            vector <float> zvukV;
            zvukAL.ToFloatArray(zvukV);
            float *zvukF = &zvukV[0];

            SF_INFO wavInfo;
            wavInfo.channels = 1;
            wavInfo.samplerate = 16000;
            wavInfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

            string soundClass;
            cout << "Please enter sound class name:" << endl;
            cin >> soundClass;

            SNDFILE *wavFile = sf_open((par.soundFolder + to_string(++lastNUM) + "_" + soundClass + ".wav").c_str(), SFM_WRITE, &wavInfo);
            soundListOUT << to_string(lastNUM) + "_" + soundClass << endl;

            sf_writef_float(wavFile, &zvukF[0], zvukV.size());
            sf_write_sync(wavFile);
            sf_close(wavFile);

            char c = 'a';
            while (c != 'n' && c != 'y'){
                cout << "Record new sound? (y/n)" << endl;
                cin >> c;
            }
            if (c == 'n') rec = false;
        }
        soundListOUT.flush();
        soundListOUT.close();
      }
      catch (const AL::ALError& e) {
        std::cerr << e.what() << std::endl;
      }
}
