#ifndef FEATURES_H
#define FEATURES_H

#include <iostream>
#include <vector>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include <xtract/libxtract.h>
#include <xtract/xtract_stateful.h>
#include <xtract/xtract_scalar.h>
#include <xtract/xtract_helper.h>

/*
 * Features that are useful and can be extracted by this module:
 *  Scalar features:
 *      - [a] - ZCR (Zero cross rating)
 *      - [b] - HZCRR (high zero cross rate ratio)
 *      - [c] - kurtosis
 *      - [d] - skewness
 *      - [e] - Spectral mean
 *      - [f] - Spectral variance
 *      - [g] - Spectral deviation
 *      - [h] - Spectral centroid
 *      - [i] - Spectral kurtosis
 *      - [j] - Spectral skewness
 *      - [k] - Sharpness
 *      - [l] - Loudness
 *      - [m] - RMS (root mean square)
 *  Vector features:
 *      - [n] - LPCC (linear predictive cepstral coefficients)
 *      - [o] - MFCC (mel-frequency cepstral coefficients)
 *      - [p] - Bark coeficcients
 */

using namespace std;

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

class soundFeatures
{
private:
    int subWindow;
    long wavSamples;
    double *wavData;
    int samplerate;

    map<char, vector<double> > result;
public:
    soundFeatures();
    ~soundFeatures();

    void setData(double *data, long n, int window, int samp);
    void clearFeatures();

    map<char, vector<double> > calcFeatures (string , int windowStart, int windowLength);

};

#endif // FEATURES_H
