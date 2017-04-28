#include "fft.h"
#include <cmath>

#define PI atan(1.0f)*4.0f
#define I sqrt(-1)

complex<double> *dft(complex<double> f[],int n)
{
    complex<double> *c;
    c=new complex<double>[n];

    for(int k=0; k<n; k++)
    {
        c[k]=0;
        for(int j=0; j<n; j++)
        {
            c[k]+=f[j]*exp((I*2*PI*j*k)/n);
        }
        c[k]/=n;
    }

    return c;
}

complex<double> *fft(complex<double> samples[],int len)
{
    if(len==1)
    {
        return new complex<double>(samples[0].real(),samples[0].imag());
    }

    complex<double> even[len/2];
    complex<double> *q;
    for(int i=0;i<len/2;i++)
        even[i]=samples[2*i];

    q=fft(even,len/2);

    complex<double> odd[len/2];
    complex<double> *r;
    //odd=even
    for(int i=0;i<len/2;i++)
        odd[i]=samples[2*i+1];

    r=fft(odd,len/2);

    complex<double> *ret=new complex<double>[len];
    for(int i=0;i<len/2;i++)
    {
        double ith=-2*i*PI/len;
        complex<double> wi=complex<double>(cos(ith),sin(ith));
        ret[i]=q[i]+(wi*r[i]);
        ret[i+len/2]=q[i]-(wi*r[i]);
    }
    delete q;
    delete r;

    return ret;
}

complex<double> *ifft(complex<double> samples[],int len)
{
    complex<double> *ret=new complex<double>[len];
    for(int i=0;i<len;i++){
        //complex<double> t=conj(samples[i]);
        //ret[i]=complex<double>(t.real(),t.imag());
        ret[i]=conj(samples[i]);
    }

    ret=fft(ret,len);

    for(int i=0;i<len;i++)
        ret[i]=conj(ret[i])*(1.0/len);

    return ret;
}

complex<double> *rfft(uint8_t samples[],int len)
{
    if(len==1)
    {
        return new complex<double>((unsigned)samples[0],0);

    }

    uint8_t even[len/2];
    complex<double> *q;
    for(int i=0;i<len/2;i++)
        even[i]=samples[2*i];

    q=rfft(even,len/2);

    uint8_t odd[len/2];
    complex<double> *r;
    //odd=even;
    for(int i=0;i<len/2;i++)
        odd[i]=samples[2*i+1];

    r=rfft(odd,len/2);

    complex<double> *ret=new complex<double>[len];
    for(int i=0;i<len/2;i++)
    {
        double ith=-2*i*PI/len;
        complex<double> wi=complex<double>(cos(ith),sin(ith));
        ret[i]=q[i]+(wi*r[i]);
        ret[i+len/2]=q[i]-(wi*r[i]);
    }
    delete q;
    delete r;
    return ret;
}

uint8_t *irfft(complex<double> samples[],int len)
{
    complex<double> *tmp=new complex<double>[len];
    uint8_t *ret=new uint8_t[len];
    for(int i=0;i<len;i++)
    {
        //complex<double> t=conj(samples[i]);
        //ret[i]=complex<double>(t.real(),t.imag());
        tmp[i]=conj(samples[i]);
    }

    tmp=fft(tmp,len);

    for(int i=0;i<len;i++)
        tmp[i]=conj(tmp[i])*(1.0/len);

    for(int i=0;i<len;i++)
        ret[i]=(tmp[i].real()+0.5);

    delete [] tmp;

    return ret;
}

complex<double> *rfft16(int16_t samples[],int len)
{
    if(len==1)
    {
        return new complex<double>(samples[0],0);
    }

    int16_t even[len/2];
    complex<double> *q;
    for(int i=0;i<len/2;i++)
        even[i]=samples[2*i];

    q=rfft16(even,len/2);

    int16_t odd[len/2];
    complex<double> *r;
    //odd=even;
    for(int i=0;i<len/2;i++)
        odd[i]=samples[2*i+1];

    r=rfft16(odd,len/2);

    complex<double> *ret=new complex<double>[len];
    for(int i=0;i<len/2;i++)
    {
        double ith=-2*i*PI/len;
        complex<double> wi=complex<double>(cos(ith),sin(ith));
        ret[i]=q[i]+(wi*r[i]);
        ret[i+len/2]=q[i]-(wi*r[i]);
    }
    delete[] q;
    delete[] r;
    return ret;
}

int16_t *irfft16(complex<double> samples[],int len)
{
    complex<double> *tmp=new complex<double>[len];
    int16_t *ret=new int16_t[len];
    for(int i=0;i<len;i++)
    {
        //complex<double> t=conj(samples[i]);
        //ret[i]=complex<double>(t.real(),t.imag());
        tmp[i]=conj(samples[i]);
    }

    complex<double> *tmp2=tmp;
    tmp=fft(tmp,len);
    delete[] tmp2;

    for(int i=0;i<len;i++)
        tmp[i]=conj(tmp[i])*(1.0/len);

    for(int i=0;i<len;i++)
        if(tmp[i].real()<0)
            ret[i]=(tmp[i].real()-0.5);
        else ret[i]=(tmp[i].real()+0.5);

    delete[] tmp;

    return ret;
}
