#include "fft_c.h"
#include <cmath>

#define PI atan(1.0f)*4.0f
#define I sqrt(-1)


std::complex<double> *dft(std::complex<double> f[], int n)
{
    std::complex<double> *c;
    c = new std::complex<double>[n];

    for(int k=0; k<n; k++)
    {
        c[k] = 0;
        for(int j=0; j<n; j++)
        {
            c[k] += f[j]*exp((I*2*PI*j*k)/n);
        }
        c[k] /= n;
    }

    return c;
}


std::complex<double> *fft(std::complex<double> samples[], int len)
{
    if(len == 1)
    {
        return new std::complex<double>(samples[0].real(),samples[0].imag());
    }

    std::complex<double> even[len/2];
    std::complex<double> *q;
    for(int i=0; i<len/2; i++)
        even[i] = samples[2*i];

    q = fft(even,len/2);

    std::complex<double> odd[len/2];
    std::complex<double> *r;
    //odd=even
    for(int i=0;i<len/2;i++)
        odd[i]=samples[2*i+1];

    r=fft(odd,len/2);

    std::complex<double> *ret=new std::complex<double>[len];
    for(int i=0;i<len/2;i++)
    {
        double ith=-2*i*PI/len;
        std::complex<double> wi = std::complex<double>(cos(ith),sin(ith));
        ret[i] = q[i] + (wi*r[i]);
        ret[i+len/2] = q[i] - (wi*r[i]);
    }
    delete q;
    delete r;

    return ret;
}

std::complex<double> *rfft8(uint8_t samples[], int len)
{
    if(len == 1)
    {
        return new std::complex<double>((unsigned)samples[0], 0);
    }

    uint8_t even[len/2];
    std::complex<double> *q;
    for(int i=0; i<len/2; i++)
        even[i] = samples[2*i];

    q = rfft8(even,len/2);

    uint8_t odd[len/2];
    std::complex<double> *r;
    //odd=even;
    for(int i=0; i<len/2; i++)
        odd[i] = samples[2*i+1];

    r=rfft8(odd,len/2);

    std::complex<double> *ret=new std::complex<double>[len];
    for(int i=0; i<len/2; i++)
    {
        double ith =- 2*i*PI/len;
        std::complex<double> wi = std::complex<double>(cos(ith), sin(ith));
        ret[i] = q[i]+(wi*r[i]);
        ret[i+len/2] = q[i] - (wi*r[i]);
    }
    delete q;
    delete r;
    return ret;
}

std::complex<double> *rfft16(int16_t samples[], int len)
{
    if(len == 1)
    {
        return new std::complex<double>(samples[0], 0);
    }

    int16_t even[len/2];
    std::complex<double> *q;
    for(int i=0; i<len/2; i++)
        even[i] = samples[2*i];

    q = rfft16(even, len/2);

    int16_t odd[len/2];
    std::complex<double> *r;
    //odd=even;
    for(int i=0; i<len/2; i++)
        odd[i] = samples[2*i+1];

    r = rfft16(odd, len/2);

    std::complex<double> *ret=new std::complex<double>[len];
    for(int i=0;i<len/2;i++)
    {
        double ith =- 2*i*PI/len;
        std::complex<double> wi = std::complex<double>(cos(ith), sin(ith));
        ret[i] = q[i] + (wi*r[i]);
        ret[i+len/2] = q[i] - (wi*r[i]);
    }
    delete [] q;
    delete [] r;
    return ret;
}

std::complex<double> *rfft32(float samples[], int len)
{
    if(len == 1)
    {
        return new std::complex<double>(samples[0], 0);
    }

    float even[len/2];
    std::complex<double> *q;
    for(int i=0; i<len/2; i++)
        even[i] = samples[2*i];

    q = rfft32(even, len/2);

    float odd[len/2];
    std::complex<double> *r;
    //odd=even;
    for(int i=0; i<len/2; i++)
        odd[i] = samples[2*i+1];

    r = rfft32(odd, len/2);

    std::complex<double> *ret=new std::complex<double>[len];
    for(int i=0;i<len/2;i++)
    {
        double ith =- 2*i*PI/len;
        std::complex<double> wi = std::complex<double>(cos(ith), sin(ith));
        ret[i] = q[i] + (wi*r[i]);
        ret[i+len/2] = q[i] - (wi*r[i]);
    }
    delete [] q;
    delete [] r;
    return ret;
}


std::complex<double> *ifft(std::complex<double> samples[], int len)
{
    std::complex<double> *ret=new std::complex<double>[len];
    for(int i=0; i<len; i++){
        //complex<double> t=conj(samples[i]);
        //ret[i]=complex<double>(t.real(),t.imag());
        ret[i] = conj(samples[i]);
    }

    ret = fft(ret, len);

    for(int i=0; i<len; i++)
        ret[i] = conj(ret[i])*(1.0/len);

    return ret;
}

uint8_t *irfft8(std::complex<double> samples[], int len)
{
    std::complex<double> *tmp = new std::complex<double>[len];
    uint8_t *ret = new uint8_t[len];
    for(int i=0; i<len; i++)
    {
        //complex<double> t=conj(samples[i]);
        //ret[i]=complex<double>(t.real(),t.imag());
        tmp[i] = conj(samples[i]);
    }

    tmp = fft(tmp, len);

    for(int i=0; i<len; i++)
        tmp[i] = conj(tmp[i])*(1.0/len);

    for(int i=0; i<len; i++)
        ret[i] = (tmp[i].real()+0.5);

    delete [] tmp;

    return ret;
}

int16_t *irfft16(std::complex<double> samples[], int len)
{
    std::complex<double> *tmp = new std::complex<double>[len];
    int16_t *ret = new int16_t[len];
    for(int i=0; i<len; i++)
    {
        //complex<double> t=conj(samples[i]);
        //ret[i]=complex<double>(t.real(),t.imag());
        tmp[i] = conj(samples[i]);
    }

    std::complex<double> *tmp2 = tmp;
    tmp = fft(tmp, len);
    delete[] tmp2;

    for(int i=0; i<len; i++)
        tmp[i] = conj(tmp[i])*(1.0/len);

    for(int i=0; i<len; i++)
        if(tmp[i].real()<0)
            ret[i] = (tmp[i].real()/*-0.5*/);
        else ret[i] = (tmp[i].real()/*+0.5*/);

    delete [] tmp;

    return ret;
}

float *irfft32(std::complex<double> samples[], int len)
{
    std::complex<double> *tmp = new std::complex<double>[len];
    float *ret = new float[len];
    for(int i=0; i<len; i++)
    {
        //complex<double> t=conj(samples[i]);
        //ret[i]=complex<double>(t.real(),t.imag());
        tmp[i] = conj(samples[i]);
    }

    std::complex<double> *tmp2 = tmp;
    tmp = fft(tmp, len);
    delete[] tmp2;

    for(int i=0; i<len; i++)
        tmp[i] = conj(tmp[i])*(1.0/len);

    for(int i=0; i<len; i++)
        if(tmp[i].real()<0)
            ret[i] = (tmp[i].real()/*-0.5*/);
        else ret[i] = (tmp[i].real()/*+0.5*/);

    delete [] tmp;

    return ret;
}
