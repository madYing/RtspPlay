#ifndef FFMPEGDECODE_H
#define FFMPEGDECODE_H

extern "C"
{
#include <include/libavcodec/avcodec.h>
#include <include/libavformat/avformat.h>
#include <include/libavfilter/avfilter.h>
#include <include/libswscale/swscale.h>
}

#include <QObject>
#include <QMutex>
#include <QImage>

class ffmpegDecode : public QObject
{
     Q_OBJECT
public:
    ffmpegDecode(QObject *parent = 0);
    ~ffmpegDecode();

    void SetUrl(QString url){this->url=url;}
    bool init();
    void h264Decodec();
    bool initRecord();
    void setRecordState(bool state);
    void wFileTrailer();
    void delRecord();
    bool flag;

signals:
    void GetImage(const QImage &image);

private:
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVFrame *pFrame;
    AVPacket packet;
    AVPicture  pAVPicture;
    SwsContext * pSwsCtx;

    AVOutputFormat *ofmt;
    AVFormatContext *o_pFormatCtx;
   // AVStream *o_vstream;

    int videoStreamIndex;
    int width;
    int height;

    QMutex mutex;
    QString url;
    QString outFileName;
    bool isRecord;
};

#endif // FFMPEGDECODE_H
