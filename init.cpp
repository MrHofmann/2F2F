#include "init.h"
#include "low_pass.h"
#include "high_pass.h"
#include "filter.h"
#include <math.h>
#include <libavutil/opt.h>


int open_file(const char *filename)
{
//    AVCodecParameters *codec_param;
    AVCodec *audio_codec;
    AVIOInterruptCB int_cb = { decode_interrupt_cb, NULL };

    initialize();

//OPEN FILE
    if(avformat_open_input(&format_context,filename, NULL, NULL) < 0)
    {
        cerr << "Error open file" << endl;
        return -1;
    }

    format_context->interrupt_callback = int_cb;

//ISPISIVANJE INFORMACIJA FORMATA
    cout << endl;
    av_dump_format(format_context,0,filename,false);

//FIND STREAM - prikuplja podatke o strimu
    if(avformat_find_stream_info(format_context, NULL) < 0)
    {
        cerr << "Error file stream info" << endl;
        return -2;
    }

//NALAZI STREAM
    av_index = av_find_best_stream(format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &audio_codec, 0);
    if(av_index<0)
    {
        cerr << "Error find best audio stream" << endl;
        return -3;
    }

//FIND DECODER - nalazi odgovarajuci decoder    
//    codec_param = format_context->streams[av_index]->codecpar;
//    audio_codec = avcodec_find_decoder(codec_param->codec_id);
//    if(!audio_codec)
//    {
//        cerr << "Unsuported codec" << endl;
//        return -4;
//    }

//    codec_context = avcodec_alloc_context3(audio_codec);
//    if(!codec_context)
//    {
//        cerr << "Failed to allocate codec context" << endl;
//        return -5;
//    }


    codec_context = format_context->streams[av_index]->codec;
//    av_opt_set_int(codec_context, "refcounted_frames", 1, 0);

//INIT DEKODER - prikupljanje informacija o kodeku
    if(avcodec_open2(codec_context, audio_codec, NULL) < 0)
    {
        cerr << "Error decoder initialization" << endl;
        return -6;
    }


    cout << endl << "FILE OPEN SUCCESS" << endl << endl;
    return 0;
}

int packet_queue_put(const AVPacket &pkt)
{
    AVPacket *tmp = av_packet_alloc();
    av_init_packet(tmp);
    if(av_packet_ref(tmp, &pkt) < 0)
        return -1;

SDL_LockMutex(mutex);

    que.push_back(tmp);
    que_nb_packets++;
    que_size += pkt.size;

    SDL_CondSignal(cond);

SDL_UnlockMutex(mutex);

    return 0;
}

int packet_queue_get(AVPacket *pkt, int block)
{
    pkt_counter++;
    int ret;

SDL_LockMutex(mutex);
    while(true)
    {
        if(sdl_quit)
        {
            ret = -1;
            break;
        }

        if(!que.empty())
        {
            auto tmp = que.front();
            if(av_packet_ref(pkt, tmp) < 0)
                return -1;

            av_packet_unref(tmp);

            que.pop_front();
            que_nb_packets--;
            que_size -= pkt->size;

            ret=1;
            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
            SDL_CondWait(cond, mutex);
    }
SDL_UnlockMutex(mutex);

    return ret;
}

void audio_callback(void *userdata, Uint8 *stream, int len)
{
    //Uint8 *stream - Bafer odakle zvucna cita podatke
    //int len - Velicina stream bafera

    AVCodecContext *aCodecCtx = (AVCodecContext *)userdata; //Codec informacija
    int len1;//Velicina niza koji se salje zvucnoj
    int audio_size;//Broj dekodiranih bajtova koje vraca audio_decode_frame funkcija
    static int len2;

    static uint8_t audio_buf[131072];//Niz koji cuva dekodirane(raw) podatke
    static unsigned int audio_buf_size = 0;//Duzina niza koji cuva dekodirane(raw) podatke
    static unsigned int audio_buf_index = 0;//Indeks koji oznacava koliko je od dekodirane informacije poslato
    static uint8_t dft_buf[DFT_AUDIO_BUFFER_SIZE];
    static uint8_t overlap_buf[DFT_AUDIO_BUFFER_SIZE];

    while(len > 0)
    {
//        cout << len << endl;
        if(audio_buf_index >= audio_buf_size)
        {
        /* We have already sent all our data; get more */
            audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));

//            cout << audio_size << endl;
            if(audio_size < 0)
            {
            /* If error, output silence */
                audio_buf_size = 1024;
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

//FFT GOES HERE
//
        if(filter_enable)
        {
            int tmp=0;
            if(len2+len1>DFT_AUDIO_BUFFER_SIZE/2)
            {
                tmp=DFT_AUDIO_BUFFER_SIZE/2-len2;
                memcpy(dft_buf + len2, audio_buf + audio_buf_index, tmp);
                len2+=tmp;
            }
            else
            {
                memcpy(dft_buf + len2, audio_buf + audio_buf_index, len1);
                len2+=len1;
            }

            if(len2==DFT_AUDIO_BUFFER_SIZE/2)
            {
                buffer_init(dft_buf,DFT_AUDIO_BUFFER_SIZE);

                if(filter_mode==LOW_PASS)
                    {
                        Low_Pass low_filter(DFT_AUDIO_BUFFER_SIZE/2);
                        low_filter.dft_filter16((int16_t *) dft_buf, DFT_AUDIO_BUFFER_SIZE/2);
                    }
                else if(filter_mode==HIGH_PASS)
                    {
                        High_Pass high_filter(DFT_AUDIO_BUFFER_SIZE/2);
                        high_filter.dft_filter16((int16_t *) dft_buf, DFT_AUDIO_BUFFER_SIZE/2);
                    }
                else if(filter_mode==CUSTOM)
                    {
                        Filter custom_filter(f_min, f_max, c_gain, DFT_AUDIO_BUFFER_SIZE/2);
                        custom_filter.dft_filter16((int16_t *) dft_buf, DFT_AUDIO_BUFFER_SIZE/2);
                    }
                else
                {

    //WINDOWING - Hamming Window
    //
                   for(int i=0; i<DFT_AUDIO_BUFFER_SIZE/4; i++)
                        ((int16_t *)dft_buf)[i]*=0.54-0.46*cos(i*2*M_PI/DFT_AUDIO_BUFFER_SIZE/4);
    //
    //WINDOWING

                    Filter equilizer(DFT_AUDIO_BUFFER_SIZE/2);
                    equilizer.set_kernel();

                    equilizer.dft_filter16((int16_t *) dft_buf, DFT_AUDIO_BUFFER_SIZE/2);
                }

    //OVERLAPPING - 50% Overlap
    //
                for(int i=0; i<len2; i++)
                {
                    overlap_buf[i]=overlap_buf[len2+i]+dft_buf[i];
                }
                    memcpy(overlap_buf+len2, dft_buf+len2, DFT_AUDIO_BUFFER_SIZE/2);
    //
    //OVERLAPPING

                memcpy(stream, overlap_buf, len2);
                stream+=len2;
                len2=0;
            }

            if(tmp!=0)
            {
                len1=tmp;
                tmp=0;
            }
                len -= len1;
                audio_buf_index += len1;

        }
        else
        {
            memcpy(stream, audio_buf + audio_buf_index, len1);

//            for(int i=0; i<len1; i++)
//                cout << (int)audio_buf[audio_buf_index+i] << " ";
//            cout << len << " len" << endl;
//            cout << len1 << " len1" << endl << endl;

            stream += len1;

            len -= len1;
            audio_buf_index += len1;
        }
//
//FFT GOES HERE

   }
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size)
{
    static AVPacket avpkt;

    AVFrame *avfrm = NULL;
    int len1, data_size = 0, return_size = 0;

    int sample_size = av_get_bytes_per_sample(aCodecCtx->sample_fmt);
    if(sample_size < 0)
    {
        /* This should not occur, checking just for paranoia */
        cerr << "Failed to calculate data size" << endl;
        exit(EXIT_FAILURE);
    }

//    cout << "INNNNNNNN" << endl << "-----------------------------------------------" << endl << endl;
    while(true)
    {
        while(avpkt.size > 0)
        {
            if(sdl_quit)
                return -1;

            int got_frame = 0;
            if(!avfrm)
                if (!(avfrm = av_frame_alloc()))
                {
                    cerr << "Could not allocate audio frame" << endl;
                    exit(EXIT_FAILURE);
                }

            len1 = avcodec_decode_audio4(aCodecCtx, avfrm, &got_frame, &avpkt);
            data_size = avfrm->nb_samples*aCodecCtx->channels*sample_size;

            if(len1 < 0)
            {
                cerr << "Error while decoding" << endl;
                break;
            }

            avpkt.size -= len1;
            avpkt.data += len1;
//            avpkt.dts =
//            avpkt.pts = AV_NOPTS_VALUE;
            if(got_frame)
            {
                unsigned i = 0;
                unsigned j = 0;
                while(i < data_size-1)
                {
                    audio_buf[i++ + return_size] = avfrm->data[0][j];
                    audio_buf[i++ + return_size] = avfrm->data[0][j+1];
                    audio_buf[i++ + return_size] = avfrm->data[1][j];
                    audio_buf[i++ + return_size] = avfrm->data[1][j+1];
                    j += 2;

//                    cout << (int)avfrm->data[0][j] << " ";
//                    cout << (int)avfrm->data[0][j+1] << " ";
//                    cout << (int)avfrm->data[1][j] << " ";
//                    cout << (int)avfrm->data[1][j+1] << " ";
                }
//                cout << endl << endl;

                buf_size -= data_size;
                return_size += data_size;

                av_frame_unref(avfrm);
            }
            else
                cout << "no frame" << endl;

            if(data_size > buf_size)
            {
//                unsigned i = 0;
//                unsigned j = 0;
//                cout << "OUTTTTTTTTTTTTT" << endl;
//                while(j < avfrm->nb_samples*sample_size)
//                {
//                    cout << (int)avfrm->data[0][j] << " ";
//                    cout << (int)avfrm->data[0][j+1] << " ";
//                    cout << (int)avfrm->data[1][j] << " ";
//                    cout << (int)avfrm->data[1][j+1] << " ";
//                    j += 2;
//                }
//                cout << data_size << " data_size" << endl;
//                cout << "-------------------------------------------------------------------" << endl << endl;
                return return_size;
            }
        }

        if(packet_queue_get(&avpkt, 1) < 0)
            return -1;
    }
}

int decode_interrupt_cb(void *)
{
  return sdl_quit;
}

int buffer_init(uint8_t *buff,int len)
{
    for(int i=len/2; i<len; i++)
    {
        buff[i]=0;
    }

    return 0;
}

void initialize()
{
    playback_enable = false;
    track_loaded = false;
    playback_stop = false;

    for(int i=0; i<10; i++)
        f_gain[i]=100;

    av_register_all();
    avcodec_register_all();

    mutex = SDL_CreateMutex();
    cond = SDL_CreateCond();
    if(SDL_Init( SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        cerr << "Error SDL initialization" << SDL_GetError() << endl;
        exit(1);
    }

    SDL_PauseAudio(1);
}
