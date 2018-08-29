#ifndef FFT_C_H
#define FFT_C_H

#include <complex>

std::complex<double> *dft(std::complex<double> f[], int n);

std::complex<double> *fft(std::complex<double> samples[], int len);
std::complex<double> *rfft8(uint8_t samples[], int len);
std::complex<double> *rfft16(int16_t samples[], int len);
std::complex<double> *rfft32(float samples[], int len);

std::complex<double> *ifft(std::complex<double> samples[], int len);
uint8_t *irfft8(std::complex<double> samples[], int len);
int16_t *irfft16(std::complex<double> samples[], int len);
float *irfft32(std::complex<double> samples[], int len);

#endif // FFT_C_H
