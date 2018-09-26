#ifndef FFT_H
#define FFT_H

#include <vector>
#include <complex>


template<typename T>
void zero_pad(std::vector<T> &x);

template<typename T>
void zero_pad(std::vector<T> &x, unsigned power);

template<typename T>
void zero_pad(std::vector<T> &x, std::vector<T> &h);

template<typename T>
void zero_pad(std::vector<T> &x, std::vector<std::complex<double> > &f);

template<typename T>
void window(std::vector<T> &samples);


template<typename T>
std::complex<double> *fft_cpp(const T samples[], unsigned len);

std::complex<double> *ifft_cpp(const std::complex<double> samples[], unsigned len);

int16_t *ifft16_cpp(std::complex<double> samples[], unsigned len);

//FIR
std::vector<std::complex<double> > neutral(unsigned n);
//FIR
std::vector<std::complex<double> > zero(unsigned n);
//FIR
std::vector<std::complex<double> > butterworth_lp(unsigned n, double fs, unsigned order, double cutoff, double dc_gain);
//FIR
std::vector<std::complex<double> > butterworth_hp(unsigned n, double fs, unsigned order, double cutoff, double dc_gain);
//IIR - hardcoded
void butterworth(double cutoff, double rate, double &b0, double &b1, double &b2, double &a1, double &a2);


//NUMERICKE METODE - circular convolution
template<typename T>
std::vector<T> convolution_circ(const std::vector<T> &x, const std::vector<T> &h);
//DSP GUIDE - output side algorithm (linear convolution)
template<typename T>
std::vector<T> convolution_out(const std::vector<T> &x, const std::vector<T> &h);
//DSP GUIDE - input side algorithm (linear convolution)
template<typename T>
std::vector<T> convolution_in(const std::vector<T> &x, const std::vector<T> &h);


template<typename T>
std::vector<T> convolution_fft(const std::vector<T> &x, const std::vector<T> &h);

std::vector<int16_t> convolution_fft16(std::vector<int16_t> &x, std::vector<std::complex<double> > &hf);
template<typename T>
std::vector<T> convolution_fft_circ(const std::vector<T> &x, const std::vector<T> &h);


std::vector<int16_t> overlap_add_conv(const std::vector<int16_t> &x, const std::vector<int16_t> &h);

std::vector<int16_t> overlap_add_fft(const std::vector<int16_t> &x, const std::vector<int16_t> &h);

#endif // FFT_H
