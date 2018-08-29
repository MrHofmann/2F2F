#include "datastructures.h"

void packet_queue_init(PacketQueue *queue)
{
    queue->mutex = SDL_CreateMutex();
    queue->cond = SDL_CreateCond();
    queue->nb_packets = 0;
    queue->size = 0;
}

int packet_queue_put(PacketQueue *queue, const AVPacket &pkt)
{
    AVPacket *tmp = av_packet_alloc();
    av_init_packet(tmp);
    if(av_packet_ref(tmp, &pkt) < 0)
        return -1;

SDL_LockMutex(queue->mutex);

    queue->queue.push_back(tmp);
    queue->nb_packets++;
    queue->size += pkt.size;

    SDL_CondSignal(queue->cond);

SDL_UnlockMutex(queue->mutex);

    return 0;
}

int packet_queue_get(PacketQueue *queue, AVPacket *pkt, int block, TrackState *track_state)
{
    queue->pkt_counter++;
    int ret;

SDL_LockMutex(queue->mutex);
    while(true)
    {
        if(track_state->sdl_quit)
        {
            ret = -1;
            break;
        }

        if(!queue->queue.empty())
        {
            auto tmp = queue->queue.front();
            if(av_packet_ref(pkt, tmp) < 0)
                return -1;

            av_packet_unref(tmp);

            queue->queue.pop_front();
            queue->nb_packets--;
            queue->size -= pkt->size;

            ret = 1;
            break;
        }
        else if(!block)
        {
            ret = 0;
            break;
        }
        else
            SDL_CondWait(queue->cond, queue->mutex);
    }
SDL_UnlockMutex(queue->mutex);

    return ret;
}

