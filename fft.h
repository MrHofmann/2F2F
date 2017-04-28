#ifndef FFT_H
#define FFT_H
#include <complex>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

using namespace std;

complex<double> *dft(complex<double> f[],int n);
complex<double> *fft(complex<double> samples[],int len);
complex<double> *ifft(complex<double> samples[],int len);
complex<double> *rfft(uint8_t samples[],int len);
uint8_t *irfft(complex<double> samples[],int len);

complex<double> *rfft16(int16_t samples[],int len);
int16_t *irfft16(complex<double> samples[],int len);

#endif // FFT_H
