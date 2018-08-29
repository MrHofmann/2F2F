#include "decodethread.h"

#include <SDL2/SDL_timer.h>
#include <iostream>


#define MAX_audio_queue_SIZE (5 * 16 * 1024)


DecodeThread::DecodeThread(TrackState *state)
    :QThread(), _track_state(state)
{
}

void DecodeThread::run()
{
    AVPacket packet;

    while(true)
    {
        if(_track_state->sdl_quit == 1)
            break;

        if (_track_state->audio_queue.size > MAX_audio_queue_SIZE)
        {
//            SDL_Delay(10);
            continue;
        }

        if(av_read_frame(_track_state->format_context, &packet) < 0 )
        {
            if (_track_state->format_context->pb->error == 0)
            {
                SDL_Delay(100);
                continue;
            }
            else
                break;
        }

        if(packet.stream_index == _track_state->av_index)
            packet_queue_put(&(_track_state->audio_queue), packet);
        else
            av_packet_unref(&packet);

//        SDL_PauseAudio(0);
    }

    while (!_track_state->sdl_quit)
        SDL_Delay(100);

    emit decoded();
}

int DecodeThread::decode_interrupt_cb(void *)
{
  return _track_state->sdl_quit;
}
