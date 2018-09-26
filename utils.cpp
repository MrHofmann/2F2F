#include "utils.h"
#include "filtah.h"
#include "fft.h"
#include <math.h>
#include <assert.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif


template<typename T>
void update_volume(std::vector<std::vector<T> > &splitted_channels, unsigned master_volume, unsigned balance)
{
    for(unsigned i=0; i<splitted_channels.size(); i++)
        for(unsigned j=0; j<splitted_channels[0].size(); j++)
            splitted_channels[i][j] *= std::pow(10, (-48+(54.0*master_volume)/100)/20);

    if(splitted_channels.size() == 2)
        for(unsigned i=0; i<splitted_channels[0].size(); i++)
        {
            splitted_channels[0][i] *= (2 - (balance/100.0));
            splitted_channels[1][i] *= balance/100.0;
        }
}

template<typename T>
void split_channels(T *audio_buf, int len, unsigned num_channels, std::vector<std::vector<T> > &splitted_channels)
{
    for(unsigned i=0; i<num_channels; i++)
        splitted_channels.push_back(std::vector<T>());

    for(int i=0; i<len/num_channels; i++)
        for(unsigned j=0; j<num_channels; j++)
        {
            splitted_channels[j].push_back(audio_buf[num_channels*i + j]);
        }

}

template<typename T>
void merge_channels(const std::vector<std::vector<T> > &splitted_channels, T *audio_buf, int len)
{
    unsigned num_channels = splitted_channels.size();
    for(int i=0; i<len/num_channels; i++)
        for(unsigned j=0; j<num_channels; j++)
        {
            audio_buf[num_channels*i + j] = splitted_channels[j][i];
        }
}


void audio_callback(void *userdata, Uint8 *stream, int len)
{
    TrackState *track_state = ((UserData*)userdata)->track_state;
    FilterState *filter_state = ((UserData*)userdata)->filter_state;
    AVCodecContext *codec_ctx = track_state->codec_context; //Codec informacija

    unsigned num_channels = codec_ctx->channels;
    unsigned sample_size = track_state->audio_sample_size;
    int len1;//Velicina niza koji se salje zvucnoj
    int audio_size;//Broj dekodiranih bajtova koje vraca audio_decode_frame funkcija
    static int len2;

//    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3)/2];
    static uint8_t audio_buf[FFT_AUDIO_BUFFER_SIZE];//Niz koji cuva dekodirane(raw) podatke
    static unsigned int audio_buf_size = 0;//Duzina niza koji cuva dekodirane(raw) podatke
    static unsigned int audio_buf_index = 0;//Indeks koji oznacava koliko je od dekodirane informacije poslato
    static uint8_t fft_buf[FFT_AUDIO_BUFFER_SIZE];

    while(len > 0)
    {
        if(audio_buf_index >= audio_buf_size)
        {
        /* We have already sent all our data; get more */
            audio_size = audio_decode_frame(track_state, audio_buf, sizeof(audio_buf));

//            cout << audio_size << endl;
            if(audio_size < 0)
            {
            /* If error, output silence */
                audio_buf_size = 1024;
                memset(audio_buf, 0, audio_buf_size);
                std::cout << "silence" << std::endl;
            }
            else
                audio_buf_size = audio_size;

            audio_buf_index = 0;
        }

        len1 = audio_buf_size - audio_buf_index;
        if(len1 > len)
            len1 = len;

//FFT GOES HERE
//
        int tmp=0;
        if(len2+len1 > FFT_AUDIO_BUFFER_SIZE)
        {
            tmp = FFT_AUDIO_BUFFER_SIZE - len2;
            memcpy(fft_buf + len2, audio_buf + audio_buf_index, tmp);
            len2 += tmp;
        }
        else
        {
            memcpy(fft_buf + len2, audio_buf + audio_buf_index, len1);
            len2+=len1;
        }

        if(len2 == FFT_AUDIO_BUFFER_SIZE)
        {
            std::vector<std::complex<double> > freqs;
            std::vector<std::vector<float> > splitted_channels;
            split_channels((float *)fft_buf, len2/sample_size, num_channels, splitted_channels);


            Filter *filter = filter_state->filters[0];
            filter->set_kernel_size(len2/(num_channels*sample_size));
            filter->set_sample_rate(44100);
//                filter->update_kernel();

    //AUDIO PROCESSING
    //
            filter_state->frequencies.clear();
//                update_volume(splitted_channels, filter_state->master_volume, filter_state->balance);
            for(unsigned i=0; i<num_channels; i++)
            {
//                    window(splitted_channels[i]);
                freqs.clear();
                splitted_channels[i] = filter->convolve(splitted_channels[i], freqs, filter_state->filter_method);

                if(filter_state->frequencies.empty() == false)
                {
                    for(unsigned i=0; i<freqs.size(); i++)
                        filter_state->frequencies[i] += freqs[i];
                }
                else
                    filter_state->frequencies = freqs;
            }

            for(unsigned i=0; i<filter_state->frequencies.size(); i++)
                filter_state->frequencies[i] /= num_channels;
    //
    //AUDIO PROCESSING

            //RENDER HERE

            merge_channels(splitted_channels, (float *)fft_buf, len2/sample_size);
            memcpy(stream, fft_buf, len2);

            stream += len2;
            len2 = 0;
        }

        if(tmp != 0)
        {
            len1 = tmp;
            tmp = 0;
        }

        len -= len1;
        audio_buf_index += len1;
//
//FFT GOES HERE
   }
}

int audio_decode_frame(TrackState *track_state, uint8_t *audio_buf, int buf_size)
{
    AVCodecContext *codec_ctx = track_state->codec_context;
    AVFrame *avfrm = nullptr;
    static AVPacket avpkt;
    int len1, return_size = 0;
    int sample_size, data_size;


    while(true)
    {
        while(avpkt.size > 0)
        {
            if(track_state->sdl_quit)
                return -1;

            int got_frame = 0;
            if(!avfrm)
                if (!(avfrm = av_frame_alloc()))
                {
                    std::cerr << "Could not allocate audio frame" << std::endl;
                    exit(EXIT_FAILURE);
                }

            len1 = avcodec_decode_audio4(codec_ctx, avfrm, &got_frame, &avpkt);
            if(len1 < 0)
            {
                std::cerr << "Error while decoding" << std::endl;
                break;
            }

            avpkt.size -= len1;
            avpkt.data += len1;
//            avpkt.dts =
//            avpkt.pts = AV_NOPTS_VALUE;
            if(got_frame)
            {
                sample_size = track_state->audio_sample_size;
                data_size = avfrm->nb_samples*codec_ctx->channels*sample_size;

                int size1 = sample_size*codec_ctx->channels;
                int size2 = sample_size;
                for(unsigned i=0; i<avfrm->nb_samples; i++)
                    for(unsigned c=0; c<codec_ctx->channels; c++)
                        for(unsigned j=0; j<sample_size; j++)
                            audio_buf[i*size1 + c*size2 + j + return_size] = avfrm->data[c][i*size2 + j];

                buf_size -= data_size;
                return_size += data_size;

                av_frame_unref(avfrm);
            }
            else
                std::cout << "no frame" << std::endl;

            if(data_size > buf_size)
            {
                return return_size;
            }
        }

//        int get = packet_queue_get(&(track_state->audio_queue), &avpkt, 1, track_state);
        if(packet_queue_get(&(track_state->audio_queue), &avpkt, 1, track_state) < 0)
            return -1;
    }
}


void audio_callback2(void *userdata, Uint8 *stream, int len)
{
    AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
    int len1, audio_size;

    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    while(len > 0)
    {
        if(audio_buf_index >= audio_buf_size)
        {
            /* We have already sent all our data; get more */
//            audio_size = audio_decode_frame2(aCodecCtx, audio_buf, sizeof(audio_buf));
            if(audio_size < 0)
            {
                /* If error, output silence */
                audio_buf_size = 1024; // arbitrary?
                memset(audio_buf, 0, audio_buf_size);
            }
            else
            {
                audio_buf_size = audio_size;
            }

            audio_buf_index = 0;
        }

        len1 = audio_buf_size - audio_buf_index;
        if(len1 > len)
            len1 = len;

        memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

int audio_decode_frame2(TrackState *track_state, uint8_t *audio_buf, int buf_size)
{
    AVCodecContext *codec_ctx = track_state->codec_context;
    static AVFrame avfrm;
    static AVPacket avpkt;
    static uint8_t *audio_pkt_data = nullptr;
    static int audio_pkt_size = 0;

    int len1, data_size = 0;

    int sample_size = av_get_bytes_per_sample(codec_ctx->sample_fmt);
    if(sample_size < 0)
    {
        /* This should not occur, checking just for paranoia */
        std::cerr << "Failed to calculate data size" << std::endl;
        exit(EXIT_FAILURE);
    }

    while(true)
    {
        while(avpkt.size > 0)
        {
            if(track_state->sdl_quit)
                return -1;

            int got_frame = 0;
            len1 = avcodec_decode_audio4(codec_ctx, &avfrm, &got_frame, &avpkt);
//            data_size = avfrm.nb_samples * codec_ctx->channels * sample_size;

            if(len1 < 0)
            {
                std::cerr << "Error while decoding, skipping frame" << std::endl;
                audio_pkt_size = 0;
                break;
            }

//            avpkt.size -= len1;
//            avpkt.data += len1;
            audio_pkt_data += len1;
            audio_pkt_size -= len1;

            data_size = 0;
            if(got_frame)
            {
                data_size = av_samples_get_buffer_size(nullptr, codec_ctx->channels, avfrm.nb_samples, codec_ctx->sample_fmt, 1);
                assert(data_size <= buf_size);
                memcpy(audio_buf, avfrm.data[0], data_size);
            }
            else
                std::cout << "Error, no frame" << std::endl;

            if(data_size <= 0)
            {
                /* No data yet, get more frames */
                continue;
            }

            /* We have data, return it and come back for more later */
            return data_size;
        }

        if(avpkt.data)
            av_free_packet(&avpkt);

        if(packet_queue_get(&(track_state->audio_queue), &avpkt, 1, track_state) < 0)
        {
            std::cout << "Packet queue get" << std::endl;
            return -1;
        }
        audio_pkt_data = avpkt.data;
        audio_pkt_size = avpkt.size;
    }
}



/*
        if(filter_state->filter_enabled == false || filter_state->filters.size() == 0)
        {
            memcpy(stream, audio_buf + audio_buf_index, len1);

            stream += len1;
            len -= len1;
            audio_buf_index += len1;
        }

  */




/*else
{
//            std::cout << len1/(num_channels*sample_size) << std::endl;
    memcpy(fft_buf, audio_buf + audio_buf_index, len1);

    std::vector<std::vector<float> > splitted_channels;
    split_channels((float *)fft_buf, len1/sample_size, num_channels, splitted_channels);

    //INITIALIZE FILTERS HERE
    Filter *filter = filter_state->filters[0];
    filter->set_kernel_size(len1/(sample_size*num_channels));
    filter->set_sample_rate(44100);
    filter->update_kernel();

//AUDIO PROCESSING
//
//            update_volume(splitted_channels, len2/sample_size);
    for(unsigned i=0; i<num_channels; i++)
    {
//                window(splitted_channels[i], len2/sample_size);
        splitted_channels[i] = filter->convolve(splitted_channels[i], filter_state->filter_method);
    }
//
//AUDIO PROCESSING

    merge_channels(splitted_channels, (float *)fft_buf, len1/sample_size);
    memcpy(stream, fft_buf, len1);

    stream += len1;
    len -= len1;
    audio_buf_index += len1;
}*/

/*if(filter_method == CONV)
{
    std::vector<int16_t> filter(len1/4, 0);

    std::vector<std::vector<int16_t> > splitted_channels;
    split_channels((int16_t *)fft_buf, len1/2, 2, splitted_channels);

//AUDIO PROCESSING
//
//            update_volume(splitted_channels, len1/4);

    std::vector<std::vector<int16_t> > convolved_bufs;
    static std::vector<std::vector<int16_t> > overlap_bufs;
    for(unsigned i=0; i<2; i++)
    {
//                window(splitted_channels[i], len2/4);

        std::vector<int16_t> convolved_buf;
        std::vector<int16_t> send_buf;
        std::vector<int16_t> overlap_buf;

        convolved_buf = convolution_in(splitted_channels[i], filter);
        for(unsigned i=0; i<convolved_buf.size(); i++)
        {
            if(i < len1/4)
                send_buf.push_back(convolved_buf[i]);
            else
                overlap_buf.push_back(convolved_buf[i]);
        }

        convolved_bufs.push_back(send_buf);
        overlap_bufs.push_back(overlap_buf);
    }
//
//AUDIO PROCESSING

    merge_channels(splitted_channels, (int16_t *)fft_buf, len1/2);
    memcpy(stream, fft_buf, len1);

    len -= len1;
    audio_buf_index += len1;
}
else if(filter_method == OA_CONV)
{
    memcpy(fft_buf, audio_buf + audio_buf_index, len1);

    std::vector<std::vector<int16_t> > splitted_channels;
    split_channels((int16_t *)fft_buf, len1/2, num_channels, splitted_channels);

//            double power = std::ceil(std::log2(splitted_channels[0].size() * 1.5));
    unsigned kernel_size = 64;
//            std::vector<std::complex<double> > kernel = butterworth_lp(kernel_size, 44100, 20, 10000, 1);
    std::vector<std::complex<double> > kernel = neutral(kernel_size);

    int16_t *fil = ifft16_cpp(kernel.data(), kernel_size);
    std::vector<int16_t> filter;
    for(unsigned i=0; i<kernel_size; i++)
    {
        filter.push_back(fil[i]);
//                std::cout << fil[i] << " ";
    }
//            std::cout << std::endl;

    static std::vector<std::vector<int16_t> > overlaps(num_channels, std::vector<int16_t>(kernel_size-1));
    for(unsigned i=0; i<num_channels; i++)
    {
//                  window(splitted_channels[i], len1/4);
        std::vector<int16_t> input = splitted_channels[i];
        std::vector<int16_t> output = convolution_in(input, filter);

        for(unsigned j=0; j<output.size(); j++)
        {
            if(j < kernel_size-1)
                splitted_channels[i][j] = output[j] + overlaps[i][j];
            else if(j < input.size())
                splitted_channels[i][j] = output[j];
            else overlaps[i][j-input.size()] = output[j];
        }
    }

    merge_channels(splitted_channels, (int16_t *)fft_buf, len1/2);
    memcpy(stream, fft_buf, len1);

    stream += len1;
    len -= len1;
    audio_buf_index += len1;
}
else if(filter_method == OA_FFT)
{
    memcpy(fft_buf, audio_buf + audio_buf_index, len1);

    std::vector<std::vector<int16_t> > splitted_channels;
    split_channels((int16_t *)fft_buf, len1/2, 2, splitted_channels);

    unsigned kernel_size = 64;
    static std::vector<std::vector<int16_t> > overlaps(num_channels, std::vector<int16_t>(kernel_size-1));
    for(unsigned i=0; i<num_channels; i++)
    {
//                  window(splitted_channels[i], len1/4);
        std::vector<int16_t> input = splitted_channels[i];

        unsigned input_size = input.size();
        unsigned power = std::ceil(std::log2(input_size + kernel_size - 1));

        std::vector<std::complex<double> > kernel = butterworth_lp(std::pow(2, power), 44100, 20, 100, 1);
//                std::vector<std::complex<double> > kernel = neutral(std::pow(2, power));
//                std::vector<std::complex<double> > kernel = zero(std::pow(2, power));
        zero_pad(input, power);

        std::vector<int16_t> output = convolution_fft(input, kernel);

        for(unsigned j=0; j<output.size(); j++)
        {
//                    if(j < kernel_size-1)
//                        splitted_channels[i][j] = output[j] + overlaps[i][j];
//                    else if(j < input_size)
//                        splitted_channels[i][j] = output[j];
//                    else if(j-input_size < kernel_size - 1)
//                        overlaps[i][j-input_size] = output[j];
        }
    }

    merge_channels(splitted_channels, (int16_t *)fft_buf, len1/2);
    memcpy(stream, fft_buf, len1);

    stream += len1;
    len -= len1;
    audio_buf_index += len1;
}
*/

/*
memcpy(fft_buf, audio_buf + audio_buf_index, len1);

std::vector<std::vector<float> > splitted_channels;
split_channels((float *)fft_buf, len1/sample_size, num_channels, splitted_channels);
for(unsigned i=0; i<splitted_channels.size(); i++)
{
    std::vector<float> input = splitted_channels[i];
}

merge_channels(splitted_channels, (float *)fft_buf, len1/sample_size);
memcpy(stream, fft_buf, len1);

stream += len1;
len -= len1;
audio_buf_index += len1;*/

/*
 * switch(filter_state->filters[0]->type())
                {
                    case BaseFilter::EQUALIZER:{
                        Equalizer *eq = (Equalizer*)(filter_state->filters[0]);
                        eq->set_f_gain(filter_state->f_gain);
                        eq->set_kernel_size(len1/(sample_size*num_channels));
                        eq->set_sample_rate(codec_ctx->sample_rate);
                        eq->update_kernel();
                        break;
                    }
                    case BaseFilter::LOW_PASS:
                    {
                        LowPass *low = (LowPass*)(filter_state->filters[0]);
                        low->set_order(filter_state->order);
                        low->set_kernel_size(len1/(sample_size*num_channels));
                        low->set_sample_rate(codec_ctx->sample_rate);
                        low->set_cutoff(1000.0);
                        low->set_gain(1.0);
                        low->update_kernel();
                        break;
                    }
                    case BaseFilter::HIGH_PASS:
                    {
                        HighPass *high = (HighPass*)(filter_state->filters[0]);
                        high->set_order(filter_state->order);
                        high->set_kernel_size(len1/(sample_size*num_channels));
                        high->set_sample_rate(codec_ctx->sample_rate);
                        high->set_cutoff(1000.0);
                        high->set_gain(1.0);
                        high->update_kernel();
                        break;
                    }
                    case BaseFilter::BAND_PASS:
                    {
                        BandPass *band = (BandPass*)(filter_state->filters[0]);
                        band->set_order(filter_state->order);
                        band->set_kernel_size(len1/(sample_size*num_channels));
                        band->set_sample_rate(codec_ctx->sample_rate);
                        band->set_mean(1000.0);
                        band->set_width(200.0);
                        band->set_gain(1.0);
                        band->update_kernel();
                        break;
                    }
                    default:
                        break;
                }
*/

/*
 *                 switch(filter_state->filters[0]->type())
                {
                    case BaseFilter::EQUALIZER:
                    {
                        std::cout << "EQUALIZER" << std::endl;
                        break;
                    }
                    case BaseFilter::LOW_PASS:
                    {
                        std::cout << "LOW_PASS" << std::endl;
                        break;
                    }
                    case BaseFilter::HIGH_PASS:
                    {
                        std::cout << "HIGH_PASS" << std::endl;
                        break;
                    }
                    case BaseFilter::BAND_PASS:
                    {
                        std::cout << "BAND_PASS" << std::endl;
                        break;
                    }
                    default:
                        break;
                }
                */
