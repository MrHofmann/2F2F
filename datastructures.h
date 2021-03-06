#ifndef DATASTRUCTURES_H
#define DATASTRUCTURES_H

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_audio.h>
#include <list>
#include <complex>
#include "filtah.h"

#define SDL_AUDIO_BUFFER_SIZE 32768
#define FFT_AUDIO_BUFFER_SIZE 65536
//#define SDL_AUDIO_BUFFER_SIZE 8192
//#define FFT_AUDIO_BUFFER_SIZE 16384
#define MAX_AUDIO_FRAME_SIZE 192000


typedef struct PacketQueue{
    std::list<AVPacket*> queue;
    int size;
    int nb_packets;
    int pkt_counter;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;


typedef struct TrackState{
    PacketQueue audio_queue;
    SDL_AudioSpec   wanted_spec, spec;  //MOZDA TREBA DA SE IZBACI
    AVFormatContext *format_context;
    AVCodecContext *codec_context;

    AVStream        *audio_st;
    AVPacket        audio_pkt;

    int av_index;                       //MOZDA TREBA STATIC
    int sdl_quit;
    int audio_sample_size;
    int audio_hw_buf_size;
    unsigned audio_buf_size;
    unsigned audio_buf_index;

    SDL_mutex *mutex;
    SDL_cond *cond;
} TrackState;


typedef struct FilterState{
    std::vector<Filter*> filters;
    std::vector<std::complex<double> > frequencies;

    Filter::Type filter_type;
    Filter::Method filter_method;

    bool filter_enabled;
    bool filter_initialized;

    std::vector<int> f_gain;
    double dc_gain;
    unsigned order;
    double cutoff;

    double cutoff1;
    double cutoff2;
    double width;

    unsigned master_volume;
    unsigned balance;

} FilterState;


typedef struct UserData{
    TrackState *track_state;
    FilterState *filter_state;
} UserData;


void packet_queue_init(PacketQueue *queue);
int packet_queue_put(PacketQueue *queue, const AVPacket &pkt);
int packet_queue_get(PacketQueue *queue, AVPacket *pkt, int block, TrackState *track_state);

#endif // DATASTRUCTURES_H
