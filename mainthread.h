#ifndef MAINTHREAD_H
#define MAINTHREAD_H

#include <QThread>
#include "datastructures.h"

class MainThread : public QThread
{
    Q_OBJECT

public:
    MainThread(TrackState *state);
    void run();

private:
    TrackState *_track_state;

Q_SIGNALS:
    void finished();


};

#endif // MAINTHREAD_H
