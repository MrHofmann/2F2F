#include "mainthread.h"
#include "datastructures.h"
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>

#define FF_ALLOC_EVENT (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)


MainThread::MainThread(TrackState *state)
    :QThread(), _track_state(state)
{

}

void MainThread::run()
{
    SDL_Event event;

    av_register_all();
    avcodec_register_all();

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS))
    {
        std::cerr << "Error SDL initialization: " << SDL_GetError() << std::endl;
        exit(1);
    }

//    av_strlcpy(videoState->filename, argv[1], sizeof(videoState->filename));

    _track_state->mutex = SDL_CreateMutex();
    _track_state->cond = SDL_CreateCond();

    //ovo ne treba kad postoji samo audio
//    schedule_refresh(track_state, 100);

    //ovde je decode thread
    SDL_PauseAudio(1);

    while(true)
    {
//        if(_playback_enable)

//        SDL_WaitEvent(&event);
        SDL_PollEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                _track_state->sdl_quit = 1;

                SDL_CondSignal(_track_state->audio_queue.cond);

                SDL_Quit();
                exit(0);

                break;
            case SDL_AUDIO_PAUSED:
                std::cout << "AUDIO PAUSED" << std::endl;
                break;
            case SDL_AUDIO_STOPPED:
//                _track_state->sdl_quit = 1;
//                SDL_Quit();
//                std::cout << "AUDIO STOPPED" << std::endl;
                break;
            case FF_ALLOC_EVENT:
//                alloc_picture(event.user.data1);
                break;
            case FF_REFRESH_EVENT:
//                video_refresh_timer(event.user.data1);
                break;
            default:
                break;
        }
        SDL_Delay(20);
    }

    emit finished();
}
