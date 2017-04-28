#include "init.h"
#include "filter.h"
#define DFT_BUFFER_SIZE 32768
#define DEBUG_ENABLE 0


Filter::Filter(int len)
    :f_min(20), f_max(22000), gain(1.0), kernel_size(len/2)
{
    kernel=new double[len/2];
}

Filter::Filter(double min, double max, double g, int len)
{
    f_min=min;
    f_max=max;
    gain=g;
    kernel=new double[len];
    kernel_size=len;

    for(int i=0; i<kernel_size/2; i++)
    {
        double freq=i*(codec_context->sample_rate/(len*1.0));
        if(freq>=min && freq<max)
            kernel[i]=gain/100.0;
        else
            kernel[i]=1.0;
    }
}

Filter::~Filter()
{
    delete [] kernel;
}


void Filter::set_kernel()
{
    for(int i=0; i<kernel_size; i++)
    {
        double freq=i*(codec_context->sample_rate/(kernel_size*1.0));
        if(freq<64)
            kernel[i]=(f_gain[0]/100.0);
        else if(freq<128)
            kernel[i]=(f_gain[1]/100.0);
        else if(freq<256)
            kernel[i]=(f_gain[2]/100.0);
        else if(freq<512)
            kernel[i]=(f_gain[3]/100.0);
        else if(freq<1024)
            kernel[i]=(f_gain[4]/100.0);
        else if(freq<2048)
            kernel[i]=(f_gain[5]/100.0);
        else if(freq<4096)
            kernel[i]=(f_gain[6]/100.0);
        else if(freq<8192)
            kernel[i]=(f_gain[7]/100.0);
        else if(freq<16383)
            kernel[i]=(f_gain[8]/100.0);
        else if(freq<20000)
            kernel[i]=(f_gain[9]/100.0);
        else
            kernel[i]=0;
    }

}

void Filter::dft_filter16(int16_t *samples,int len)
{
    if(DEBUG_ENABLE)
    {
        for(int i=0;i<len;i++)
            cout << samples[i] << " ";
        cout << endl << endl;
    }

    int16_t left_samples[len/2];
    int16_t right_samples[len/2];
    int16_t isamples[len];

    int16_t *left_isamples;
    int16_t *right_isamples;

    complex<double> *left_fsamples;
    complex<double> *right_fsamples;

    for(int i=0; i<len/2; i++)
    {
        left_samples[i]=samples[2*i];
        right_samples[i]=samples[2*i+1];
        left_samples[i]*=(master_volume/20.0)*(2-(balance/100.0));
        right_samples[i]*=(master_volume/20.0)*(balance/100.0);
    }

    left_fsamples=rfft16(left_samples,len/2);
    right_fsamples=rfft16(right_samples,len/2);



//AUDIO PROCESSING
//
    for(int i=0; i<len/2; i++)
    {
  //     double freq=i*(codec_context->sample_rate/(len*1.0));
  //      if(freq>=f_min && freq<=f_max /*i>((CUTOFF_FREQ*len/2)/codec_context->sample_rate)*2.0*/)
        {
            left_fsamples[i]*=kernel[i];
            right_fsamples[i]*=kernel[i];
        }
    }
//
//AUDIO PROCESSING


    left_isamples=irfft16(left_fsamples,len/2);
    right_isamples=irfft16(right_fsamples,len/2);


    for(int i=0; i<len/2; i++)
    {
        isamples[2*i]=left_isamples[i];
        isamples[2*i+1]=right_isamples[i];
    }

    memcpy(samples,isamples,len);

    delete[]  left_fsamples;
    delete[]  right_fsamples;
    delete[]  left_isamples;
    delete[]  right_isamples;

    if(DEBUG_ENABLE)
    {
        for(int i=0;i<len;i++)
            cout << samples[i] << " ";
        cout<< "-------------------------------------------------------------------------------------" << endl;

    }


/* DEBUG_2
            for(int i=0;i<len;i++)
                  if(samples[i]!=isamples[i])
                      cout << samples[i] << " == " << isamples[i] << endl;
*/

}

void Filter::dft_filter(uint8_t *samples, int len)
{
        if(len==DFT_BUFFER_SIZE)
        {

            cout << "dobro je" << endl;

            uint8_t left_samples[len/2];
            uint8_t right_samples[len/2];
            uint8_t isamples[len];

            uint8_t *left_isamples;
            uint8_t *right_isamples;

            complex<double> *left_fsamples;
            complex<double> *right_fsamples;

            for(int i=0, j=0; i<len/4,j<len/2; i++, j+=2)
            {
                left_samples[j]=samples[4*i];
                left_samples[j+1]=samples[4*i+1];
                right_samples[j]=samples[4*i+2];
                right_samples[j+1]=samples[4*i+3];
            }

            left_fsamples=rfft(left_samples,len/2);
            right_fsamples=rfft(right_samples,len/2);

    //AUDIO PROCESSING
    //

         /*   for(int i; i<len/2; i++){
            double freq=i*(codec_context->sample_rate/(len/2*1.0));
                if(freq>CUTOFF_FREQ){
                    //left_fsamples[i]=complex<double>(0,0);
                    //right_fsamples[i]=complex<double>(0,0);
                }
            }
        */
    //
    //AUDIO PROCESSING

            left_isamples=irfft(left_fsamples,len/2);
            right_isamples=irfft(right_fsamples,len/2);

            for(int i=0, j=0;i<len/4, j<len/2; i++, j+=2)
            {
                isamples[4*i]=left_isamples[j];
                isamples[4*i+1]=left_isamples[j+1];
                isamples[4*i+2]=right_isamples[j];
                isamples[4*i+3]=right_isamples[j+1];
            }

            memcpy(samples,isamples,len);

            delete[]  left_fsamples;
            delete[]  right_fsamples;
            delete[]  left_isamples;
            delete[]  right_isamples;
        }
        else cout << "Pogresna duzina bafera" << endl;


}
