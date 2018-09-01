#include "filtah.h"
#include "fft.h"

#include <iostream>
#include <complex>

#define DEBUG_ENABLED 0


template const std::vector<float> Filter::convolve<float>(const std::vector<float> signal, Method method) const;

template<typename T>
const std::vector<T> Filter::convolve(const std::vector<T> signal, Method method) const
{
    static bool channel = false;
    std::vector<T> output = std::vector<T>(signal.size());
    static std::vector<T> overlap = std::vector<T>(_kernel_size-1);
    static std::vector<T> overlap1 = std::vector<T>(_kernel_size-1);
    static std::vector<T> overlap2 = std::vector<T>(_kernel_size-1);

    std::vector<std::complex<double> > kernel = std::vector<std::complex<double> >(_kernel_size);
    for(unsigned i=0; i<_kernel_size; i++)
        kernel[i] = std::complex<double>(_kernel[i], 0);

    if(method == FFT)
    {
        std::vector<std::complex<double> > kernel = std::vector<std::complex<double> >(_kernel_size);
        for(unsigned i=0; i<_kernel_size; i++)
            kernel[i] = std::complex<double>(_kernel[i], 0);

        std::complex<double> *ikernel = ifft_cpp(kernel.data(), kernel.size());
        delete [] ikernel;

        std::vector<T> h = std::vector<T>(kernel.size());
        for(unsigned i=0; i<h.size(); i++)
            h[i] = (T)ikernel[i].real();

        std::vector<T> out = convolution_fft(signal, h);

        for(unsigned i=0; i<signal.size()+_kernel.size()-1; i++)
        {
            if(i < output.size())
                output[i] = out[i];
            else
                overlap[i-output.size()] = out[i];
        }
    }
    else if(method == CONV)
    {
        std::complex<double> *ikernel = ifft_cpp(kernel.data(), kernel.size());
        delete [] ikernel;

        std::vector<T> h = std::vector<T>(kernel.size());
        for(unsigned i=0; i<h.size(); i++)
            h[i] = (T)ikernel[i].real();

        std::vector<T> out = convolution_in(signal, h);
    }
    else if(method == OA_FFT)
    {
//        std::cout << "OA_FFT" << std::endl;
        std::vector<T> x = signal;
        std::vector<T> y;

        zero_pad(x, kernel);

        std::complex<double> *xi;
        std::complex<double> *xf;

        xf = fft_cpp(x.data(), x.size());

    //AUDIO PROCESSING
    //
        for(unsigned i=0; i<x.size(); i++)
            xf[i] *= kernel[i];
    //
    //AUDIO PROCESSING

        xi = ifft_cpp(xf, x.size());

//        for (unsigned i=0; i<signal.size(); i++)
//            output[i] = isamples[i].real();

        for(unsigned i=0; i<signal.size()+_kernel.size()-1; i++)
        {
            if(!channel)
            {
                if(i < _kernel_size-1)
                    output[i] = xi[i].real()+overlap1[i];
                else if(i < signal.size())
                    output[i] = xi[i].real();
                else
                    overlap1[i-signal.size()] = xi[i].real();
            }
            else
            {
                if(i < _kernel_size-1)
                    output[i] = xi[i].real()+overlap2[i];
                else if(i < signal.size())
                    output[i] = xi[i].real();
                else
                    overlap2[i-signal.size()] = xi[i].real();
            }
        }

        delete[]  xf;
        delete[]  xi;
    }
    else if(method == OA_CONV)
    {
        std::cout << "OA_CONV" << std::endl;

        std::complex<double> *ikernel = ifft_cpp(kernel.data(), kernel.size());
        delete [] ikernel;

        std::vector<T> h = std::vector<T>(kernel.size());
        for(unsigned i=0; i<h.size(); i++)
            h[i] = (T)ikernel[i].real();

        std::vector<T> out = convolution_in(signal, h);

        for(unsigned i=0; i<signal.size()+_kernel.size()-1; i++)
        {
            if(i < output.size())
                output[i] = out[i];
            else
                overlap[i-output.size()] = out[i];
        }
    }
    else
    {
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

    channel = !channel;

    return output;
}



Filter::Filter(unsigned kernel_size, double sample_rate)
    :_kernel_size(kernel_size), _sample_rate(sample_rate)
{

}

std::vector<double> Filter::kernel() const
{
    return _kernel;
}

double Filter::sample_rate() const
{
    return _sample_rate;
}

void Filter::set_kernel_size(unsigned kernel_size)
{
    _kernel_size = kernel_size;
}

void Filter::set_sample_rate(double sample_rate)
{
    _sample_rate = sample_rate;
}



Equalizer::Equalizer(unsigned size, double sample_rate, std::vector<int> *f_gain)
    :Filter(size, sample_rate), _f_gain(f_gain)
{
    update_kernel();
}

LowPass::LowPass(unsigned kernel_size, double sample_rate, unsigned *order, double *cutoff, double *gain)
    :Filter(kernel_size, sample_rate), _order(order), _cutoff(cutoff), _gain(gain)
{
    update_kernel();
}

HighPass::HighPass(unsigned kernel_size, double sample_rate, unsigned *order, double *cutoff, double *gain)
    :Filter(kernel_size, sample_rate), _order(order), _cutoff(cutoff), _gain(gain)
{
    update_kernel();
}

BandPass::BandPass(unsigned kernel_size, double sample_rate, unsigned *order, double *cutoff1, double *cutoff2, double *width, double *gain)
    :Filter(kernel_size, sample_rate), _order(order), _cutoff1(cutoff1), _cutoff2(cutoff2), _width(width), _gain(gain)
{
    update_kernel();
}



Filter::Type Equalizer::type() const
{
return EQUALIZER;
}

Filter::Type LowPass::type() const
{
return LOW_PASS;
}

Filter::Type HighPass::type() const
{
return HIGH_PASS;
}

Filter::Type BandPass::type() const
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
    butter_lp = butterworth_lp(_kernel_size, _sample_rate, *_order, *_cutoff1, *_gain);
    std::vector<std::complex<double> > butter_hp;
    butter_hp = butterworth_hp(_kernel_size, _sample_rate, *_order, *_cutoff2, *_gain);

    _kernel = std::vector<double>(_kernel_size);
    for(unsigned i=0; i<_kernel_size; i++)
        _kernel[i] = butter_lp[i].real()*butter_hp[i].real();

//    for(unsigned i=0; i<_kernel_size/2; i++)
//        std::cout << butter_lp[i].real() << " ";
//    std::cout << std::endl << std::endl;

//    for(unsigned i=0; i<_kernel_size/2; i++)
//        std::cout << butter_hp[i].real() << " ";
//    std::cout << std::endl << std::endl;

//    for(unsigned i=0; i<_kernel_size/2; i++)
//        std::cout << _kernel[i] << " ";
//    std::cout << std::endl;
}
