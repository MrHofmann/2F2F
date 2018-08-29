#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include "datastructures.h"
#include <QThread>

class DecodeThread : public QThread
{
    Q_OBJECT

public:
    DecodeThread(TrackState *state);
    void run();

private:
    int decode_interrupt_cb(void *);

    TrackState *_track_state;

Q_SIGNALS:
    void decoded();
};

#endif // DECODETHREAD_H
