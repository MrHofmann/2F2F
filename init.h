#ifndef INIT_H
#define INIT_H

#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <list>

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

#define SDL_AUDIO_BUFFER_SIZE 32768
#define DFT_AUDIO_BUFFER_SIZE 65536
#define EQUILIZER 0
#define LOW_PASS 1
#define HIGH_PASS 2
#define CUSTOM 3


using namespace std;


extern list<AVPacket*> que;
extern int que_nb_packets;
extern int que_size;
extern SDL_mutex *mutex;
extern SDL_cond *cond;

extern SDL_AudioSpec   wanted_spec, spec;
extern AVFormatContext *format_context;
extern AVCodecContext *codec_context;

extern bool playback_enable;
extern bool playback_stop;
extern bool track_loaded;
extern bool filter_enable;

extern int pkt_counter;
extern int sdl_quit;
extern int av_index;
extern int filter_mode;
extern int master_volume;
extern int balance;
extern int f_min;
extern int f_max;
extern int c_gain;
extern int f_gain[10];


void initialize();
int open_file(const char *filename);
int buffer_init(uint8_t *buff,int len);

void packet_queue_init();
int packet_queue_put(const AVPacket &pkt);
int packet_queue_get(AVPacket *pkt, int block);/*bilo je static ranije*/

void audio_callback(void *userdata, Uint8 *stream, int len);
int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size);
int decode_interrupt_cb(void *);
int dft_filter(uint8_t *samples, int len);
int  avcodec_decode_audio2(AVCodecContext *avctx, int16_t *samples,
                          int *frame_size_ptr,
                          uint8_t *buf, int buf_size);


#endif // INIT_H
