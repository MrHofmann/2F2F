#include "fft.h"
#include "fft_c.h"
#include <iostream>



template std::complex<double> *fft_cpp<float>(const float samples[], unsigned len);

template void zero_pad<int16_t>(std::vector<int16_t> &x, unsigned power);



template<typename T>
void zero_pad(std::vector<T> &x)
{
    double power = std::ceil(std::log2(x.size()));
    while(x.size() < std::pow(2, power))
        x.push_back(0);
}

template<typename T>
void zero_pad(std::vector<T> &x, unsigned power)
{
    while(x.size() < std::pow(2, power))
        x.push_back(0);
}

template<typename T>
void zero_pad(std::vector<T> &x, std::vector<T> &h)
{
//    double power = std::ceil(std::log2(std::max(x.size(), h.size())));
    double power = std::ceil(std::log2(x.size() + h.size()));
    while(x.size() < std::pow(2, power))
        x.push_back(0);
    while(h.size() < std::pow(2, power))
        h.push_back(0);
}


template<typename T>
std::complex<double> *fft_cpp(const T samples[], unsigned len)
{
    if(len == 1)
        return new std::complex<double>(samples[0]);

    T even[len/2];
    std::complex<double> *q;
    for(unsigned i=0; i<len/2; i++)
        even[i] = samples[2*i];

    q = fft_cpp(even, len/2);

    T odd[len/2];
    std::complex<double> *r;
    //odd=even
    for(unsigned i=0; i<len/2; i++)
        odd[i] = samples[2*i+1];

    r = fft_cpp(odd, len/2);

    std::complex<double> *ret = new std::complex<double>[len];
    for(unsigned i=0; i<len/2; i++)
    {
        double ith = -2*i*M_PI/len;
        std::complex<double> wi = std::complex<double>(cos(ith), sin(ith));
        ret[i] = q[i] + (wi*r[i]);
        ret[i+len/2] = q[i] - (wi*r[i]);
    }
    delete[] q;
    delete[] r;

    return ret;
}

std::complex<double> *ifft_cpp(const std::complex<double> samples[], unsigned len)
{
    std::complex<double> *ret = new std::complex<double>[len];

    std::complex<double> tmp1[len];
    for(unsigned i=0; i<len; i++)
        tmp1[i] = conj(samples[i]);

    std::complex<double> *tmp2 = fft_cpp(tmp1, len);
    for(unsigned i=0; i<len; i++)
        tmp2[i] = conj(tmp2[i])*(1.0/len);

    for(unsigned i=0; i<len; i++)
    {
        if(tmp2[i].real()<0)
            ret[i] = (tmp2[i].real());
        else ret[i]= (tmp2[i].real());
    }

    delete[] tmp2;

    return ret;
}

int16_t *ifft16_cpp(std::complex<double> samples[], unsigned len)
{
    std::complex<double> *tmp = new std::complex<double>[len];
    int16_t *ret = new int16_t[len];
    for(unsigned i=0; i<len; i++)
    {
        //std::complex<double> t = std::conj(samples[i]);
        //ret[i] = std::complex<double>(t.real(), t.imag());
        tmp[i] = std::conj(samples[i]);
    }

    std::complex<double> *tmp2 = tmp;
    tmp = fft_cpp(tmp, len);
    delete[] tmp2;

    for(unsigned i=0; i<len; i++)
        tmp[i] = std::conj(tmp[i])*(1.0/len);

    //OVO TREBA NAPOLJE - proveri ovo bolje, sumnjivo je
    for(unsigned i=0; i<len; i++)
        if(tmp[i].real()<0)
            ret[i]=(tmp[i].real()/*-0.5*/);
        else ret[i]=(tmp[i].real()/*+0.5*/);

    delete[] tmp;

    return ret;
}


//FIR - neutral filter
std::vector<std::complex<double> > neutral(unsigned n)
{
    return std::vector<std::complex<double> >(n, std::complex<double>(1, 0));
}

//FIR - zero filter
std::vector<std::complex<double> > zero(unsigned n)
{
    return std::vector<std::complex<double> >(n, std::complex<double>(0, 0));
}

//FIR - Butterworth low-pass filter
std::vector<std::complex<double> > butterworth_lp(unsigned n, double fs, unsigned order, double cutoff, double dc_gain)
{
    unsigned num_bins = n/2;
    double bin_width = fs/n;

    std::vector<std::complex<double> > gains(n, 0);
    for(unsigned i=0; i<num_bins; i++)
    {
        double freq = bin_width * i;
        double gain = dc_gain/std::sqrt(1 + std::pow(freq/cutoff, 2.0*order));

        gains[i] = std::complex<double>(gain, 1);
        gains[n-i-1] = std::complex<double>(gain, -1);
    }

    return gains;

}

//FIR - Butterworth high-pass filter
std::vector<std::complex<double> > butterworth_hp(unsigned n, double fs, unsigned order, double cutoff, double dc_gain)
{
    unsigned num_bins = n/2;
    double bin_width = fs/n;

    std::vector<std::complex<double> > gains(n, 0);
    for(unsigned i=0; i<num_bins; i++)
    {
        double freq = bin_width * i;
        double gain = dc_gain/std::sqrt(1 + std::pow(cutoff/freq, 2.0*order));

        gains[i] = std::complex<double>(gain, 0);
        gains[n-i-1] = std::complex<double>(gain, 0);
    }

    return gains;

}

//IIR - Butterworth low-pass filter (hardcoded)
void butterworth(double cutoff, double rate, double &b0, double &b1, double &b2, double &a1, double &a2)
{
    double ff = cutoff/rate;
    const double ita = 1.0/std::tan(M_PI*ff);
    const double q = std::sqrt(2.0);

    b0 = 1.0/ (1.0 + q*ita + ita*ita);
    b1 = 2*b0;
    b2 = b0;
    a1 = 2.0*(ita*ita - 1.0)*b0;
    a2 = -(1.0 - q*ita + ita*ita) * b0;
}



//NUMERICKE METODE - circular convolution
//x = [1,2,2], y = [1,3,1], conv(x,y) = [9,7,9]
std::vector<int16_t> convolution_circ(const std::vector<int16_t> &x, const std::vector<int16_t> &h)
{
    std::vector<int16_t> y;
    for(unsigned i=0; i<x.size(); i++)
    {
        int sum = 0;
        for(unsigned j=0, k = x.size()+i; j<x.size(); j++, k--)
            sum += x[j]*h[k%x.size()];

        y.push_back(sum);
    }

    return y;
}
//DSP GUIDE - output side algorithm (linear convolution)
//x = [1,2,2], y = [1,3,1], conv(x,y) = [1,5,9,8,2]
std::vector<int16_t> convolution_out(const std::vector<int16_t> &x, const std::vector<int16_t> &h)
{
    std::vector<int16_t> y;
    int output_size = x.size() + h.size() - 1;
    for(int i=0; i<output_size; i++)
    {
        int sum = 0;
        for(int j=0; j<h.size(); j++)
            if(i-j >= 0 && i-j < x.size())
                sum += h[j]*x[i-j];

        y.push_back(sum);
    }

    return y;
}
//DSP GUIDE - input side algorithm (linear convolution)
//x = [1,2,2], y = [1,3,1], conv(x,y) = [1,5,9,8,2]
std::vector<int16_t> convolution_in(const std::vector<int16_t> &x, const std::vector<int16_t> &h)
{
    std::vector<int16_t> y;
    int output_size = x.size() + h.size() - 1;
    for(int i=0; i<output_size; i++)
        y.push_back(0);

    for(int i=0; i<x.size(); i++)
        for(int j=0; j<h.size(); j++)
            y[i+j] += x[i]*h[j];

    return y;
}

//WWW - linear fft convolution
//x0 = [1,2,2,0,0], y0 = [1,3,1,0,0], X = fft(x0), Y = fft(y0), ifft(X*Y) = [1,5,9,8,2]
//output size = input_size_1 + input_size_2 - 1
std::vector<int16_t> convolution_fft(const std::vector<int16_t> &x, const std::vector<int16_t> &h)
{
    std::vector<int16_t> x_padded = x;
    std::vector<int16_t> h_padded = h;
    zero_pad(x_padded, h_padded);

    std::complex<double> *xf = fft_cpp(x_padded.data(), x_padded.size());
    std::complex<double> *hf = fft_cpp(h_padded.data(), h_padded.size());

    std::complex<double> yf[h_padded.size()];
    for(unsigned i=0; i<h_padded.size(); i++)
        yf[i] = xf[i]*hf[i];

    std::vector<int16_t> y;

    std::complex<double> *tmp = ifft_cpp(yf, h_padded.size());
    for(unsigned i=0; i<h_padded.size(); i++)
        y.push_back(tmp[i].real());

    delete [] xf;
    delete [] hf;
    delete [] tmp;

    return y;
}

std::vector<int16_t> convolution_fft(std::vector<int16_t> &x, std::vector<std::complex<double> > &hf)
{
    std::complex<double> *xf = rfft16(x.data(), x.size());
    std::complex<double> yf[x.size()];

    for(unsigned i=0; i<x.size(); i++)
        yf[i] = xf[i]*hf[i];

    std::vector<int16_t> y;
    int16_t *tmp = irfft16(yf, x.size());
    for(unsigned i=0; i<x.size(); i++)
        y.push_back(tmp[i]);

    delete [] xf;
    delete [] tmp;

    return y;
}
//WWW - circular fft convolution
//output size = max(input size 1, input size 2)
//x = [1,2,2], y = [1,3,1], X = fft(x), Y = fft(y), ifft(X*Y) = [9,7,9]
std::vector<int16_t> convolution_fft_circ(const std::vector<int16_t> &x, const std::vector<int16_t> &h)
{
    std::vector<int16_t> x_padded = x;
    std::vector<int16_t> h_padded = h;
    zero_pad(x_padded, h_padded);

    std::complex<double> *xf = fft_cpp(x_padded.data(), x_padded.size());
    std::complex<double> *hf = fft_cpp(h_padded.data(), h_padded.size());

    std::complex<double> yf[h_padded.size()];
    for(unsigned i=0; i<h_padded.size(); i++)
        yf[i] = xf[i]*hf[i];

    std::vector<int16_t> y(std::max(x.size(), h.size()));

    std::complex<double> *tmp = ifft_cpp(yf, x_padded.size());
    for(unsigned i=0; i<x_padded.size(); i++)
    {/*
        std::cout << y[i%y.size()] << " + "
                  << tmp[i].real() << " = "
                  << y[i%y.size()] + tmp[i].real() << std::endl;*/
        y[i%y.size()] += (int16_t)tmp[i].real();
    }
    delete [] xf;
    delete [] hf;
    delete [] tmp;

    return y;
}


std::vector<int16_t> overlap_add_conv(const std::vector<int16_t> &x, const std::vector<int16_t> &h)
{
    std::vector<int16_t> result;
    std::vector<int16_t> input;
    std::vector<int16_t> output;
    static std::vector<int16_t> overlap(h.size()-1, 0);

    for(unsigned k=0; k*h.size() <= x.size(); k++)
    {
        input.clear();
        if((k+1)*h.size() <= x.size())
            std::copy(x.begin() + k*h.size(), x.begin() + (k+1)*h.size(), std::back_inserter(input));
        else
            std::copy(x.begin() + k*h.size(), x.end(), std::back_inserter(input));

//        std::copy(output.begin(), output.begin() + len, std::back_inserter(result + k*len));
        output = convolution_in(input, h);
        for(unsigned i=0; i<h.size(); i++)
            result[k*h.size() + i] = output[i];

        for(unsigned i=0; i<h.size()-1; i++)
            result[k*h.size() + i] += overlap[i];

        overlap.clear();
        std::copy(output.begin() + h.size(), output.end(), back_inserter(overlap));
    }

    return result;
}

std::vector<int16_t> overlap_add_fft(const std::vector<int16_t> &x, const std::vector<int16_t> &h)
{
    std::vector<int16_t> result;
    std::vector<int16_t> input;
    std::vector<int16_t> output;
    static std::vector<int16_t> overlap(h.size()-1, 0);

    for(unsigned k=0; k*h.size() <= x.size(); k++)
    {
        input.clear();
        if((k+1)*h.size() <= x.size())
            std::copy(x.begin() + k*h.size(), x.begin() + (k+1)*h.size(), std::back_inserter(input));
        else
            std::copy(x.begin() + k*h.size(), x.end(), std::back_inserter(input));

//        std::copy(output.begin(), output.begin() + len, std::back_inserter(result + k*len));
        output = convolution_fft(input, h);
        for(unsigned i=0; i<h.size(); i++)
            result[k*h.size() + i] = output[i];

        for(unsigned i=0; i<h.size()-1; i++)
            result[k*h.size() + i] += overlap[i];

        overlap.clear();
        std::copy(output.begin() + h.size(), output.end(), back_inserter(overlap));
    }

    return result;
}



void equilize(int16_t *audio_buf, int len)
{

}

void window(std::vector<int16_t> &audio_buf, unsigned len)
{
    for(unsigned i=0; i<len; i++)
    {
        //HAMMING
//        audio_buf[i] *= (0.54 - 0.46*cos(i*2*M_PI/(len-1)));

        //HANN
//        audio_buf[i] *= (0.5*(1 - cos(2*M_PI*i/(len-1))));

        //BLACKMAN
        audio_buf[i] *= (0.42 - 0.5*cos(2*M_PI*i/(len-1)) + 0.08*cos(4*M_PI*i/(len-1)));
    }
}
