#ifndef FILTER_H
#define FILTER_H

#include <complex>
#include <vector>
#include <iostream>
#include "fft.h"

#define DEBUG_ENABLED 0


class Filter
{
public:
    Filter(){}
    Filter(double sample_rate, unsigned len);
    ~Filter();

    void set_kernel(const std::vector<int> &f_gain);
    void set_kernel(unsigned order, double cutoff, double dc_gain);

    template<typename T>
    void fft_filter(std::vector<T> &samples) const;

private:
    double _sample_rate;

    std::vector<double> _kernel;
    unsigned _kernel_size;
};

template<typename T>
void Filter::fft_filter(std::vector<T> &samples) const
{
    if(DEBUG_ENABLED)
    {
        for(int i=0; i<samples.size(); i++)
            std::cout << samples[i] << " ";
        std::cout << std::endl << std::endl;
    }

    std::complex<double> *isamples;
    std::complex<double> *fsamples;

    fsamples = fft_cpp(samples.data(), samples.size());


//AUDIO PROCESSING
//
    for(unsigned i=0; i<samples.size(); i++)
            fsamples[i] *= _kernel[i];
//
//AUDIO PROCESSING


    isamples = ifft_cpp(fsamples, samples.size());

//    std::vector<float> tmp(isamples, isamples+samples.size());
//    samples.swap(tmp);
//    memcpy(samples, isamples,len);
//    std::copy(tmp.begin(), tmp.end(), std::back_inserter(samples));

    for(unsigned i=0; i<samples.size(); i++)
        samples[i] = isamples[i].real();

    delete[]  fsamples;
    delete[]  isamples;

    if(DEBUG_ENABLED)
    {
        for(int i=0; i<samples.size(); i++)
            std::cout << samples[i] << " ";
        std::cout << std::endl << "-------------------------------------------------------------------------------------" << std::endl
                  << "-------------------------------------------------------------------------------------" << std::endl;
    }
}


#endif // FILTER_H
