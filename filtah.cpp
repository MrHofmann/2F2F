#include "filtah.h"


template std::vector<float> BaseFilter::convolve<float>(const std::vector<float> signal, Method method) const;

template<typename T>
std::vector<T> BaseFilter::convolve(const std::vector<T> signal, Method method) const
{
    if(method == FFT)
    {

    }
    else if(method == CONV)
    {

    }
    else if(method == OA_CONV)
    {

    }
    else if(method == OA_FFT)
    {

    }
    else
    {

    }

    return signal;
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


void Equalizer::set_kernel()
{

}

void LowPass::set_kernel()
{

}

void HighPass::set_kernel()
{

}

void BandPass::set_kernel()
{

}
