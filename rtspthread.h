#ifndef RTSPTHREAD_H
#define RTSPTHREAD_H
#include <QThread>
#include "ffmpegdecode.h"

class RtspThread : public QThread
{
public:
    RtspThread();
    ~RtspThread();
    void run();
    void setFFmpeg(ffmpegDecode * ff);

private:
    ffmpegDecode * ffmpeg;
};

#endif // RTSPTHREAD_H
