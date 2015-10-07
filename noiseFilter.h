#ifndef NOISE_FILTER_H
#define NOISE_FILTER_H

class noiseFilter{
private:
    double *noiseGate;
    double decayPower;
    double **smoothGain;
	int windowSize;
    int decayCount;
    int bufferSize;
    int freqWindow;
    double gateSensitivity;
    double gateTime;

	bool initNoise;

    void rotateBuffer();
    void smoothFreq(double *smooth);
    double* insertWindow(double *window);
    double gauss(int ex, int x);

public:
    noiseFilter();
    virtual ~noiseFilter();
	
	void initFilter(int noiseSamples, double *noise);
    void reduceNoise(int soundSamples, double *sound);
};

#endif // !NOISE_FILTER_H
