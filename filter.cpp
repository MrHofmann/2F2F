#include "filter.h"

#include <iterator>
#include <iostream>
#include <string.h>


Filter::Filter(double sample_rate, unsigned len)
    :_sample_rate(sample_rate), _kernel_size(len)
{
    _kernel = std::vector<double>(len);
}


Filter::~Filter(){}

void Filter::set_kernel(const std::vector<int> &f_gain)
{
    for(unsigned i=0; i<_kernel_size; i++)
    {
        double freq_max = _sample_rate*1.0;
        double freq = i*(_sample_rate/(_kernel_size*1.0));

        if(freq<64 || freq>freq_max-64)
            _kernel[i] = f_gain[0]/100.0;
        else if(freq<128 || freq>freq_max-128)
            _kernel[i] = f_gain[1]/100.0;
        else if(freq<256 || freq>freq_max-256)
            _kernel[i] = f_gain[2]/100.0;
        else if(freq<512 || freq>freq_max-512)
            _kernel[i] = f_gain[3]/100.0;
        else if(freq<1024 || freq>freq_max-1024)
            _kernel[i] = f_gain[4]/100.0;
        else if(freq<2048 || freq>freq_max-2048)
            _kernel[i] = f_gain[5]/100.0;
        else if(freq<4096 || freq>freq_max-4096)
            _kernel[i] = f_gain[6]/100.0;
        else if(freq<8192 || freq>freq_max-8192)
            _kernel[i] = f_gain[7]/100.0;
        else if(freq<16384 || freq>freq_max-16384)
            _kernel[i] = f_gain[8]/100.0;
        else //if(freq<_sample_rate/2.0 || freq>freq_max-_sample_rate/2.0)
            _kernel[i] = f_gain[9]/100.0;
//        else
//            _kernel[i] = 0;
    }
}

void Filter::set_kernel(unsigned order, double cutoff, double dc_gain)
{
    std::vector<std::complex<double> > butter = butterworth_lp(_kernel_size, _sample_rate, order, cutoff, dc_gain);
    for(unsigned i=0; i<butter.size(); i++)
        _kernel[i] = butter[i].real();
}



/*i>((CUTOFF_FREQ*len/2)/codec_context->sample_rate)*2.0*/
