#include <cstdlib>
#include <iostream>
#include "noiseFilter.h"
#include "fft_ifft.h"

complexNum **buffer;

noiseFilter::noiseFilter(){
	initNoise = false;	
	windowSize = 2048;
}

noiseFilter::~noiseFilter(){
    delete[] noiseGate;
}

void noiseFilter::initFilter(int noiseSamples, double *noise){
    decayCount = 3;
    bufferSize = decayCount*2 + 1;
    decayPower = 0.4;
    freqWindow = 38;
    gateSensitivity = 2.5;
    gateTime = 3;

    if(gateTime > decayCount) gateTime = decayCount;

    int dataCount=2*noiseSamples/windowSize-1;
    complexNum *noiseFFT[dataCount];
    noiseGate = new double [windowSize+1];
   	
    for(int k=0, i=0; i < dataCount ; k += windowSize/2, i++)
       noiseFFT[i] = fft(zeroPad(hann(&noise[k],windowSize),windowSize), windowSize*2);
   	
    for (int k = 0; k <= windowSize; k++)
        for (int i = 0; i + gateTime -1 < dataCount; i++){
            double min = noiseFFT[0][k].avg();
            for (int j = 0; j < gateTime ; j++)
                if(noiseFFT[i + j][k].avg() < min) min = noiseFFT[i + j][k].avg();
            if (min > noiseGate[k]) noiseGate[k] = min;
        }
   	initNoise = true;
}

void noiseFilter::reduceNoise(int soundSamples, double *sound){
	if(!initNoise) return;

    int tailSize;
    buffer = new complexNum *[bufferSize];
    smoothGain = new double *[bufferSize];
    double *empty = new double [windowSize]();

    for (int k = 0; k < bufferSize; k++) {
        buffer[k] = new complexNum [windowSize*2];
        smoothGain[k] = new double [windowSize+1]();
    }

    int inPtr = 0, outPtr = 0 , iter = 0;

    for (; inPtr < soundSamples; inPtr += windowSize/2, iter++){
        double *tempOut;

        if (inPtr + windowSize > soundSamples){
            double *temp = new double[windowSize];
            int k = 0;
            tailSize = soundSamples - inPtr;
            for (; k < tailSize; k++) temp[k]= sound[inPtr + k];
            for (; k < windowSize; k++) temp[k]= 0;
            tempOut = insertWindow(temp);
            delete[] temp;
        } else tempOut = insertWindow(&sound[inPtr]);

        if (iter > bufferSize-2){
            for(int k = 0; k < windowSize/2; k++){
                sound[outPtr + k] = outPtr > 0 ? sound[outPtr+k] + tempOut[k] : tempOut[k];
                sound[outPtr + k + windowSize/2] = tempOut[k+windowSize/2];
            }
            outPtr += windowSize/2;

        }
        delete[] tempOut;
    }

    for (int k = 0; outPtr < soundSamples; k++, iter++){
        double *tempOut = insertWindow(empty);
        if (iter > bufferSize -2 && outPtr < soundSamples)
            if (outPtr + windowSize > soundSamples){
                for (int i = 0; i < tailSize; i++){
                    sound[outPtr + i] = outPtr > 0 ? sound[outPtr+i] + tempOut[i] : tempOut[i] ;
                    if(outPtr + k + windowSize/2 < soundSamples)
                        sound[outPtr + i + windowSize/2] = tempOut[i];
                }
                outPtr += windowSize;
            }else{
                for(int i = 0; i < windowSize/2; i++){
                    sound[outPtr + i] = outPtr > 0 ? sound[outPtr+i] + tempOut[i] : tempOut[i];
                    sound[outPtr + i + windowSize/2] = tempOut[i+windowSize/2];
                }
                outPtr += windowSize/2;
            }
        delete[] tempOut;
    }

    for (int k = 0; k < bufferSize; k++) {
        delete[] buffer[k];
        delete[] smoothGain[k];
    }

    delete[] buffer;
    delete[] smoothGain;
    delete[] empty;
}

double* noiseFilter::insertWindow(double *window){
    delete[] buffer[0];
    delete[] smoothGain[0];

    buffer[0] = fft(zeroPad(hann(window,windowSize),windowSize), windowSize*2);
    smoothGain[0] = new double [windowSize+1]();

    for (int k = 0; k <= windowSize; k++){
        double min = buffer[0][k].avg();
        for (int i = decayCount - gateTime; i < decayCount + gateTime ; i++)
            if (buffer[i][k].avg() < min) min = buffer[i][k].avg();
        if(min > noiseGate[k] * gateSensitivity && smoothGain[decayCount][k] < 1.0)
            smoothGain[decayCount][k] = 1.0;
        else
            smoothGain[decayCount][k] = 0.0;
    }

    for (int i = 0; i <= windowSize; i++)
        for (int j = 1; j <= decayCount; j++){
            smoothGain[decayCount+j][i] = smoothGain[decayCount+j][i] > smoothGain[decayCount+j-1][i] * decayPower ? smoothGain[decayCount+j][i] : smoothGain[decayCount+j-1][i] * decayPower;
            smoothGain[decayCount-j][i] = smoothGain[decayCount-j][i] > smoothGain[decayCount-j+1][i] * decayPower ? smoothGain[decayCount-j][i] : smoothGain[decayCount-j+1][i] * decayPower;
        }

    rotateBuffer();

    smoothFreq(smoothGain[0]);

    buffer[0][0].Re *= smoothGain[0][0];
    buffer[0][0].Im *= smoothGain[0][0];

    buffer[0][windowSize].Re *= smoothGain[0][windowSize];
    buffer[0][windowSize].Im *= smoothGain[0][windowSize];

    for (int j = 1; j < windowSize; j++){
        buffer[0][j].Re *= smoothGain[0][j];
        buffer[0][j].Im *= smoothGain[0][j];

        buffer[0][2*windowSize-j].Re *= smoothGain[0][j];
        buffer[0][2*windowSize-j].Im *= smoothGain[0][j];
    }

    return hann(ifft(buffer[0],windowSize*2),windowSize);
}

void noiseFilter::rotateBuffer(){
    complexNum *tempc = buffer[bufferSize-1];
    double *tempd = smoothGain[bufferSize-1];

    for (int k = bufferSize -1; k > 0; k--){
        buffer[k] = buffer[k-1];
        smoothGain[k] = smoothGain[k-1];
    }
    buffer[0] = tempc;
    smoothGain[0] = tempd;
}

double noiseFilter::gauss(int ex, int x){
    double va = pow(freqWindow/6, 2);
    double a = 1 /(sqrt(2*PI*va));
    double c = a * exp (- pow(x - ex, 2) / (2 * va));
  //  cout << ex << " "<< c << " " << x << endl;
    return c;
}

void noiseFilter::smoothFreq(double *smooth){
    if (freqWindow < 2) return;
    double *temp = new double[windowSize + 1]();
    for (int i = 0; i <= windowSize; i++){
        int winStart = i - freqWindow/2;
        double maxGauss = gauss(i - winStart, i - winStart);
        int j0 = i - freqWindow/2 < freqWindow? 0 : i - freqWindow/2;
        int j1 = i + freqWindow/2 < windowSize +1? i + freqWindow/2 : windowSize+1;
        for (int j =j0; j < j1 ; j++)temp[i] += smooth[j] * gauss(i - winStart, j - winStart);
        temp[i] /= 0.42;
    }
    for (int i = 0; i <= windowSize; i++){
        //cout << smooth[i] << " " << temp[i] << endl;
            smooth[i] = temp[i];}
    delete[] temp;
}

