#include "filtah.h"
#include "fft.h"

#include <iostream>
#include <complex>

#define DEBUG_ENABLED 0


template std::vector<float> BaseFilter::convolve<float>(const std::vector<float> signal, Method method) const;

template<typename T>
std::vector<T> BaseFilter::convolve(const std::vector<T> signal, Method method) const
{
    std::vector<T> output = std::vector<T>(signal.size());

    if(method == FFT)
    {
        std::cout << "FFT" << std::endl;
        if(DEBUG_ENABLED)
        {
            for(int i=0; i<signal.size(); i++)
                std::cout << signal[i] << " ";
            std::cout << std::endl << std::endl;
        }

        std::complex<double> *isamples;
        std::complex<double> *fsamples;

        fsamples = fft_cpp(signal.data(), signal.size());


    //AUDIO PROCESSING
    //
        for(unsigned i=0; i<signal.size(); i++)
                fsamples[i] *= _kernel[i];
    //
    //AUDIO PROCESSING


        isamples = ifft_cpp(fsamples, signal.size());

    //    std::vector<float> tmp(isamples, isamples+samples.size());
    //    samples.swap(tmp);
    //    memcpy(samples, isamples,len);
    //    std::copy(tmp.begin(), tmp.end(), std::back_inserter(samples));

        for(unsigned i=0; i<signal.size(); i++)
            output[i] = isamples[i].real();

        delete[]  fsamples;
        delete[]  isamples;

        if(DEBUG_ENABLED)
        {
            for(int i=0; i<signal.size(); i++)
                std::cout << signal[i] << " ";
            std::cout << std::endl << "-------------------------------------------------------------------------------------" << std::endl
                      << "-------------------------------------------------------------------------------------" << std::endl;
        }
    }
    else if(method == CONV)
    {
        std::cout << "CONV" << std::endl;
    }
    else if(method == OA_CONV)
    {
        std::cout << "OA_CONV" << std::endl;
    }
    else if(method == OA_FFT)
    {
        std::cout << "OA_FFT" << std::endl;
    }
    else
    {        
        std::cout << "NONE" << std::endl;
    }

    return output;
}


BaseFilter::BaseFilter(unsigned kernel_size, double sample_rate)
    :_kernel_size(kernel_size), _sample_rate(sample_rate)
{

}

void BaseFilter::set_kernel_size(unsigned kernel_size)
{
    _kernel_size = kernel_size;
}

void BaseFilter::set_sample_rate(double sample_rate)
{
    _sample_rate = sample_rate;
}


Equalizer::Equalizer(unsigned size, double sample_rate, std::vector<int> *f_gain)
    :BaseFilter(size, sample_rate), _f_gain(f_gain)
{
    update_kernel();
}

LowPass::LowPass(unsigned kernel_size, double sample_rate, unsigned *order, double *cutoff, double *gain)
    :BaseFilter(kernel_size, sample_rate), _order(order), _cutoff(cutoff), _gain(gain)
{
    update_kernel();
}

HighPass::HighPass(unsigned kernel_size, double sample_rate, unsigned *order, double *cutoff, double *gain)
    :BaseFilter(kernel_size, sample_rate), _order(order), _cutoff(cutoff), _gain(gain)
{
    update_kernel();
}

BandPass::BandPass(unsigned kernel_size, double sample_rate, unsigned *order, double *mean, double *width, double *gain)
    :BaseFilter(kernel_size, sample_rate), _order(order), _mean(mean), _width(width), _gain(gain)
{
    _cutoff_lp = *_mean + *_width/2;
    _cutoff_hp = *_mean - *_width/2;

    update_kernel();
}



BaseFilter::Type Equalizer::type() const
{
return EQUALIZER;
}

BaseFilter::Type LowPass::type() const
{
return LOW_PASS;
}

BaseFilter::Type HighPass::type() const
{
return HIGH_PASS;
}

BaseFilter::Type BandPass::type() const
{
return BAND_PASS;
}


void Equalizer::update_kernel()
{
    _kernel = std::vector<double>(_kernel_size);
    for(unsigned i=0; i<_kernel_size; i++)
    {
        double freq_max = _sample_rate*1.0;
        double freq = i*(_sample_rate/(_kernel_size*1.0));

        if(freq<64 || freq>freq_max-64)
            _kernel[i] = (*_f_gain)[0]/100.0;
        else if(freq<128 || freq>freq_max-128)
            _kernel[i] = (*_f_gain)[1]/100.0;
        else if(freq<256 || freq>freq_max-256)
            _kernel[i] = (*_f_gain)[2]/100.0;
        else if(freq<512 || freq>freq_max-512)
            _kernel[i] = (*_f_gain)[3]/100.0;
        else if(freq<1024 || freq>freq_max-1024)
            _kernel[i] = (*_f_gain)[4]/100.0;
        else if(freq<2048 || freq>freq_max-2048)
            _kernel[i] = (*_f_gain)[5]/100.0;
        else if(freq<4096 || freq>freq_max-4096)
            _kernel[i] = (*_f_gain)[6]/100.0;
        else if(freq<8192 || freq>freq_max-8192)
            _kernel[i] = (*_f_gain)[7]/100.0;
        else if(freq<16384 || freq>freq_max-16384)
            _kernel[i] = (*_f_gain)[8]/100.0;
        else
            _kernel[i] = (*_f_gain)[9]/100.0;
    }
}

void LowPass::update_kernel()
{
    std::vector<std::complex<double> > butter;
    butter = butterworth_lp(_kernel_size, _sample_rate, *_order, *_cutoff, *_gain);

    _kernel = std::vector<double>(_kernel_size);
    for(unsigned i=0; i<butter.size(); i++)
        _kernel[i] = butter[i].real();
}

void HighPass::update_kernel()
{
    std::vector<std::complex<double> > butter;
    butter = butterworth_hp(_kernel_size, _sample_rate, *_order, *_cutoff, *_gain);

    _kernel = std::vector<double>(_kernel_size);
    for(unsigned i=0; i<butter.size(); i++)
        _kernel[i] = butter[i].real();
}

void BandPass::update_kernel()
{
    std::vector<std::complex<double> > butter_lp;
    butter_lp = butterworth_lp(_kernel_size, _sample_rate, *_order, _cutoff_lp, *_gain);
    std::vector<std::complex<double> > butter_hp;
    butter_hp = butterworth_lp(_kernel_size, _sample_rate, *_order, _cutoff_hp, *_gain);

    _kernel = std::vector<double>(_kernel_size);
    for(unsigned i=0; i<_kernel_size; i++)
        _kernel[i] = butter_lp[i].real()*butter_hp[i].real();
}
