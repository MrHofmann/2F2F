#include "playback_thread.h"
#include "init.h"
#include <QTime>

void PlaybackThread::run()
{
    AVPacket packet;
    SDL_Event event;

    while(1)
    {
        if(playback_enable)
        {
            if(av_read_frame(format_context, &packet) <0 )
                break;
            if(playback_stop)
                break;

            SDL_PauseAudio(0);

            if(packet.stream_index == av_index)
                packet_queue_put(packet);
            else
                av_packet_unref(&packet);

            SDL_PollEvent(&event);
            switch(event.type)
            {
                case SDL_QUIT:
                    sdl_quit = 1;
                    SDL_Quit();
                    exit(0);
                    break;
                default:
                    break;
            }
            SDL_Delay(26);
        }
        else
            SDL_PauseAudio(1);

    }

    emit played();
}
