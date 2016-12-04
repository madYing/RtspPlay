#include "rtspthread.h"
#include <QDebug>

RtspThread::RtspThread()
{

}

RtspThread::~RtspThread()
{

}
void RtspThread::run()
{
    ffmpeg->h264Decodec();
    qDebug() << "the thread quit" ;
}

void RtspThread::setFFmpeg(ffmpegDecode * ff)
{
    ffmpeg=ff;
}
