#include "features.h"


soundFeatures::soundFeatures()
{

}

soundFeatures::~soundFeatures()
{

}


void soundFeatures::setData(double *data, long n, int window, int samp){
    wavData = data;
    subWindow = window;
    wavSamples = n;
    samplerate = samp;
    }

map<char, vector<double> > soundFeatures::calcFeatures(string featString, int windowStart, int windowLength){
    double mean, dev, zcr, hzcrr, kurtosis, skewness, variance, RMS, sharp, lpcc_order, loud;
    double s_var ,s_mean, s_dev, s_kur, s_skew, s_centroid;
    double spectrum[windowLength], autoCorr[windowLength+1], lpc[20], lpcc[15], mfcc[10], barkC[25];
    int bark[26];

    xtract_mel_filter mfccBank;

    map <string, bool> calcFeat;

    for(int k = 0; k < featString.size(); k++){
        switch (featString[k]){
            case 'a' :
            case 'b' :
                if(!calcFeat["zcr"]){
                    xtract[XTRACT_ZCR](wavData + windowStart, windowLength, NULL, &zcr);
                    calcFeat["zcr"] = true;
                }
                if(featString[k]== 'a') result['a'] = vector<double>{zcr};

                if(featString[k]== 'b'){
                    double tempZRC;
                    double tempRes=0;
                    double tempN= windowLength / subWindow;
                    double tempAvg= zcr;
                    for (int i=0; i < windowLength ; i+=subWindow){
                        xtract[XTRACT_ZCR](wavData+i+windowStart, subWindow, NULL, &tempZRC);
                        if (tempZRC > 1.5 * tempAvg) tempRes++;
                        }
                    hzcrr = tempRes / (2*tempN);
                    result['b'] = vector<double>{hzcrr};
                }
            break;
            case 'c':
            case 'd':
                if(!calcFeat["scalars"]){
                    xtract[XTRACT_MEAN](wavData + windowStart, windowLength, NULL, &mean);
                    xtract[XTRACT_VARIANCE](wavData + windowStart, windowLength, &mean, &variance);
                    xtract[XTRACT_STANDARD_DEVIATION](wavData + windowStart, windowLength, &variance, &dev);
                    calcFeat["scalars"] = true;
                    }
                if(featString[k]== 'c'){
                    double temp_sca[2]={mean, dev};
                    xtract[XTRACT_KURTOSIS](wavData + windowStart, windowLength, temp_sca , &kurtosis);
                    result['c'] = vector<double>{kurtosis};
                    }
                if(featString[k]== 'd'){
                    double temp_sca[2]={mean, dev};
                    xtract[XTRACT_SKEWNESS](wavData + windowStart, windowLength, temp_sca, &skewness);
                    result['d'] = vector<double>{skewness};
                    }
            break;
            case 'e':
            case 'f':
            case 'g':
            case 'h':
            case 'i':
            case 'j':
            case 'k':
                if (!calcFeat["spectrum"]){
                    double temp1[4] = {(double)samplerate/windowLength, XTRACT_MAGNITUDE_SPECTRUM, 0.0f, 0.0f};
                    xtract_init_fft(windowLength, XTRACT_SPECTRUM);
                    xtract[XTRACT_SPECTRUM](wavData + windowStart, windowLength, &temp1[0], &spectrum[0]);
                    xtract_free_fft();

                    xtract[XTRACT_SPECTRAL_MEAN](spectrum, windowLength, NULL, &s_mean);
                    xtract[XTRACT_SPECTRAL_VARIANCE](spectrum, windowLength, &s_mean, &s_var);
                    xtract[XTRACT_SPECTRAL_STANDARD_DEVIATION](spectrum, windowLength, &s_var, &s_dev);
                    calcFeat["spectrum"] = true;
                    }
                if(featString[k]== 'e') result['e'] = vector<double>{s_mean};
                if(featString[k]== 'f') result['f'] = vector<double>{s_var};
                if(featString[k]== 'g') result['g'] = vector<double>{s_dev};

                {//radi odvajanja deklaracije temp unutar samo ovog bloka
                double temp[2] = {s_mean, s_dev};
                if(featString[k]== 'h'){
                    xtract[XTRACT_SPECTRAL_CENTROID](spectrum, windowLength,  NULL, &s_centroid);
                    result['h'] = vector<double>{s_centroid};
                    }
                if(featString[k]== 'i'){
                    xtract[XTRACT_SPECTRAL_KURTOSIS](spectrum, windowLength,  temp, &s_kur);
                    result['i'] = vector<double>{s_kur};
                    }
                if(featString[k]== 'j'){
                    xtract[XTRACT_SPECTRAL_SKEWNESS](spectrum, windowLength,  temp, &s_skew);;
                    result['j'] = vector<double>{s_skew};
                    }
                if(featString[k]== 'k'){
                    xtract[XTRACT_SHARPNESS](spectrum, windowLength/2,  NULL, &sharp);
                    result['k'] = vector<double>{sharp};
                    }
                }
            break;
            case 'l':
                if (!calcFeat["spectrum"]){
                    double temp1[4] = {(double)samplerate/windowLength, XTRACT_MAGNITUDE_SPECTRUM, 0.0f, 0.0f};
                    xtract_init_fft(windowLength, XTRACT_SPECTRUM);
                    xtract[XTRACT_SPECTRUM](wavData + windowStart, windowLength, &temp1[0], &spectrum[0]);
                    xtract_free_fft();

                    calcFeat["spectrum"] = true;
                }
                xtract_init_bark(windowLength/2, samplerate, bark);
                xtract[XTRACT_BARK_COEFFICIENTS](spectrum, windowLength/2, bark, barkC);
                calcFeat["bark"] = true;

                xtract[XTRACT_LOUDNESS](barkC, 10, NULL, &loud);
                result['l'] = vector<double>{loud};
            break;
            case 'm':
                xtract[XTRACT_RMS_AMPLITUDE](wavData + windowStart, windowLength, NULL, &RMS);
                result['m'] = vector<double>{RMS};
            break;
            case 'n':
                xtract[XTRACT_AUTOCORRELATION](wavData + windowStart, windowLength, NULL, autoCorr);
                xtract[XTRACT_LPC](autoCorr, 11, NULL, lpc);
                lpcc_order = 15;
                xtract[XTRACT_LPCC](lpc + 10, 10, &lpcc_order, lpcc);
                result['n'] = vector<double>(lpcc, lpcc + (int)lpcc_order);
            break;
            case 'o':
                if (!calcFeat["spectrum"]){
                double temp1[4] = {(double)samplerate/windowLength, XTRACT_MAGNITUDE_SPECTRUM, 0.0f, 0.0f};
                xtract_init_fft(windowLength, XTRACT_SPECTRUM);
                xtract[XTRACT_SPECTRUM](wavData + windowStart, windowLength, &temp1[0], &spectrum[0]);
                xtract_free_fft();

                calcFeat["spectrum"] = true;
                }

                mfccBank.n_filters = 10;
                mfccBank.filters = (double **)malloc(10 * sizeof(double *));
                for(int i = 0; i < 10; i++)
                    mfccBank.filters[i] = (double *)malloc(windowLength * sizeof(double));

                xtract_init_mfcc(windowLength/2, samplerate/2, XTRACT_EQUAL_GAIN, 20, samplerate/2, mfccBank.n_filters, mfccBank.filters);

                xtract[XTRACT_MFCC](spectrum, windowLength/2, &mfccBank, mfcc);

                for(int i = 0; i < 10; ++i)
                    free(mfccBank.filters[i]);
                free(mfccBank.filters);

                result['o'] = vector<double>(mfcc, mfcc + 10);
            break;
            case 'p':
                if (!calcFeat["bark"]){
                    if (!calcFeat["spectrum"]){
                        double temp1[4] = {(double)samplerate/windowLength, XTRACT_MAGNITUDE_SPECTRUM, 0.0f, 0.0f};
                        xtract_init_fft(windowLength, XTRACT_SPECTRUM);
                        xtract[XTRACT_SPECTRUM](wavData + windowStart, windowLength, &temp1[0], &spectrum[0]);
                        xtract_free_fft();

                        calcFeat["spectrum"] = true;
                    }
                    xtract_init_bark(windowLength/2, samplerate, bark);
                    xtract[XTRACT_BARK_COEFFICIENTS](spectrum, windowLength/2, bark, barkC);
                    calcFeat["bark"] = true;
                    }
                result['p'] = vector<double>(barkC, barkC + 24);
            break;
        }
    }
    return result;
}
