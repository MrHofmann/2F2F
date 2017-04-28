#ifndef PLAYBACK_THREAD_H
#define PLAYBACK_THREAD_H

#include <QThread>

class PlaybackThread: public QThread
{
    Q_OBJECT

public:
    void run();

Q_SIGNALS:
    void played();
};

#endif // PLAYBACK_THREAD_H
