#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/avfft.h>
#include <libavformat/avformat.h>
}
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <QApplication>
#include <iostream>
#include <list>
#include "mainwindow.h"

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

using namespace std;

list<AVPacket*> que;
int que_size;
int que_nb_packets;

SDL_mutex *mutex;
SDL_cond *cond;

SDL_AudioSpec   wanted_spec, spec;
AVFormatContext *format_context;
AVCodecContext *codec_context;

int av_index=-1;        /*MOZDA TREBA STATIC*/
int pkt_counter=0;
int sdl_quit = 0;
int filter_mode=0;
int master_volume=100;
int balance=100;
int f_min=20;
int f_max=20000;
int c_gain=100;
int f_gain[10];

bool playback_enable=false;
bool playback_stop=false;
bool track_loaded=false;
bool filter_enable=false;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
