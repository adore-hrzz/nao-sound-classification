//Cooley-Tukey_FFT_algorithm
#ifndef FFT_IFFT_H
#define FFT_IFFT_H

#include <math.h>
#include <stdlib.h>
#include <iostream>

#ifndef PI
# define PI	3.14159265358979323846264338327950288
#endif


using namespace std;

typedef struct{
	double Re;
	double Im;

	double avg(){
		return sqrt(Re*Re + Im*Im);
    }
	
}complexNum;


complexNum* fft(complexNum *in, int n){
    complexNum *out = new complexNum[n];

	if (n==1){
		out[0] = in[0];
		return out;
	};
	
    complexNum *temp_e, *temp_o;
    complexNum *even = new complexNum[n/2];
    complexNum *odd  = new complexNum[n/2];
		
	for (int k = 0; k < n/2; k++){
		even[k] = in[2*k];
		odd[k]  = in[2*k+1];
	}
	
    temp_e = fft(even, n/2);
    temp_o = fft(odd , n/2);
		
	for(int k = 0; k < n/2; k++){
        complexNum twiddle, temp;

		twiddle.Re =  cos(2*PI*k/(double)n);
		twiddle.Im = -sin(2*PI*k/(double)n);

		temp.Re = twiddle.Re*temp_o[k].Re - twiddle.Im*temp_o[k].Im; 
		temp.Im = twiddle.Re*temp_o[k].Im + twiddle.Im*temp_o[k].Re;

		out[k+n/2].Re = (temp_e[k].Re - temp.Re);
      	out[k+n/2].Im = (temp_e[k].Im - temp.Im);

		out[k].Re = (temp_e[k].Re + temp.Re);
     	out[k].Im = (temp_e[k].Im + temp.Im);
		
     }

     delete[] temp_e;
     delete[] temp_o;
     delete[] even;
     delete[] odd;
     return out;
}



complexNum* ifft(complexNum *in, int n){
    complexNum *out = new complexNum[n];

	if (n==1){
		out[0] = in[0];
		return out;
	};
	
    complexNum *temp_e, *temp_o;
    complexNum *even = new complexNum[n/2];
    complexNum *odd  = new complexNum[n/2];

	for (int k = 0; k < n/2; k++){
		even[k] = in[2*k];
		odd[k]  = in[2*k+1];
	}
	
	temp_e = ifft(even, n/2);
	temp_o = ifft(odd , n/2);

	for(int k = 0; k < n/2; k++){
        complexNum twiddle, temp;

		twiddle.Re =  cos(2*PI*k/(double)n);
		twiddle.Im =  sin(2*PI*k/(double)n);

		temp.Re = twiddle.Re*temp_o[k].Re - twiddle.Im*temp_o[k].Im; 
		temp.Im = twiddle.Re*temp_o[k].Im + twiddle.Im*temp_o[k].Re;

		out[k].Re = (temp_e[k].Re + temp.Re)/2;
     	out[k].Im = (temp_e[k].Im + temp.Im)/2;
     	
     	out[k+n/2].Re = (temp_e[k].Re - temp.Re)/2;
      	out[k+n/2].Im = (temp_e[k].Im - temp.Im)/2;
     }

     delete[] temp_e;
     delete[] temp_o;
     delete[] even;
     delete[] odd;
     return out;
}

double* hann(double *in, int n){
    double *out= new double[n];
	
	for (int k=0; k < n; k++){
        double multiplier = 0.5 * (1 - cos(2*PI*k/(double)(n-1)));
        out[k] = in[k] * multiplier;
	}
	return out;
}

double* hann(complexNum *in, int n){
    double *out= new double[n];
	for (int k=0; k < n; k++){
		double multiplier = 0.5 * (1 - cos(2*PI*k/(double)(n-1)));
        out[k] = in[k].Re;// * multiplier;
	}
	return out;
}

complexNum* zeroPad(double *in, int n){
    complexNum *out = new complexNum[2*n];
    for(int k=0; k < n; k++){
        out[k].Re = in[k];
        out[k].Im = 0;
        out[k+n].Re = 0;
        out[k+n].Im = 0;
    }

    return out;
}

#endif // !FFT_IFFT_H
