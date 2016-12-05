#include "ffmpegdecode.h"
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>
#include "stdio.h"

//static   int   interrupt_cb( void   *ctx)

//{
//    // do something
//    time_out++;
//    if   (time_out > 40) {
//        time_out=0;
//        if   (firsttimeplay) {
//            firsttimeplay=0;
//            return   1; //这个就是超时的返回
//        }
//    }
//    return   0;
//}

ffmpegDecode::ffmpegDecode(QObject *parent):
    QObject(parent)
{
    pCodecCtx = NULL;
    videoStreamIndex=-1;
    av_register_all();//注册库中所有可用的文件格式和解码器
    avformat_network_init();//初始化网络流格式,使用RTSP网络流时必须先执行
    pFormatCtx = avformat_alloc_context();//申请一个AVFormatContext结构的内存,并进行简单初始化
    o_pFormatCtx = NULL;

    //out
    ofmt = NULL;

//   s pFormatCtx->interrupt_callback = interrupt_cb;  //注册回调函数
    pFrame=av_frame_alloc();
    //outFileName = "test.mp4";
    isRecord = false;
    flag = true;
}

ffmpegDecode::~ffmpegDecode()
{
	avformat_free_context(o_pFormatCtx);
    avformat_free_context(pFormatCtx);
    av_frame_free(&pFrame);
    sws_freeContext(pSwsCtx);
}

bool ffmpegDecode::init()
{
    //打开视频流
    int err = avformat_open_input(&pFormatCtx, url.toStdString().c_str(), NULL,NULL);
    if (err < 0)
    {
        qDebug() << "Can not open this file";
        return false;
    }

    //获取视频流信息   //读入一串流 用于分析
    //if (av_find_stream_info(pFormatCtx) < 0)
    err=avformat_find_stream_info(pFormatCtx,NULL);
    if(err < 0)
    {
        qDebug() << "Unable to get stream info";
        return false;
    }

    /*原解码部份*/
    //获取视频流索引
    videoStreamIndex = -1;
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1)
    {
        qDebug() <<"Unable to find video stream";
        return false;
    }

    //获取视频流的分辨率大小
    pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
    width=pCodecCtx->width;
    height=pCodecCtx->height;

    avpicture_alloc(&pAVPicture,PIX_FMT_RGB24,pCodecCtx->width,pCodecCtx->height);

    //获取视频流解码器
    AVCodec *pCodec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    //sws_getContext()前三个参数分别为原始图像的宽高和图像制式，4-6为目标图像的宽高和图像制式，最后三个参数为转换时使用的算法、转换时加入的filter处理
    pSwsCtx = sws_getContext(width, height, PIX_FMT_YUV420P, width,height, PIX_FMT_RGB24,SWS_BICUBIC, 0, 0, 0);
    if (pCodec == NULL)
    {
        qDebug() << "Unsupported codec";
        return false;
    }
    qDebug() << QString("video size : width=%d height=%d \n").arg(pCodecCtx->width).arg(pCodecCtx->height);

    //打开对应解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        qDebug() << "Unable to open codec";
        return false;
    }
    qDebug() << "initial successfully";

    return true;

}

bool ffmpegDecode::initRecord()
{
    /*输出初始化*/
    QString datetime = QDateTime::currentDateTime().toString("yyyy-mm-dd-hh-mm-ss");
	outFileName = datetime.append(".mp4");
    avformat_alloc_output_context2(&o_pFormatCtx, NULL, NULL, outFileName.toStdString().c_str()); //初始化输出视频码流的AVFormatContext。
    if (!o_pFormatCtx) {
        printf( "Could not create output context\n");
        return false;
    }
    ofmt = o_pFormatCtx->oformat;
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++) {
        //Create output AVStream according to input AVStream
        AVStream *in_stream = pFormatCtx->streams[i];
        AVStream *out_stream = avformat_new_stream(o_pFormatCtx, in_stream->codec->codec); //创建输出码流的AVStream。
        if (!out_stream) {
            printf( "Failed allocating output stream\n");
//            ret = AVERROR_UNKNOWN;
//            goto end;
            return false;
        }
        //Copy the settings of AVCodecContext
        if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {   //拷贝输入视频码流的AVCodecContex的数值t到输出视频的AVCodecContext。
            printf( "Failed to copy context from input to output stream codec context\n");
            //goto end;
        }
        out_stream->codec->codec_tag = 0;
        if (o_pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    //Output information------------------
    av_dump_format(o_pFormatCtx, 0, outFileName.toStdString().c_str(), 1);
    //Open output file
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        int ret = avio_open(&o_pFormatCtx->pb, outFileName.toStdString().c_str(), AVIO_FLAG_WRITE);  //打开输出文件。
        if (ret < 0) {
            printf( "Could not open output file '%s'", outFileName.toStdString().c_str());
            //goto end;
        }
    }
    //Write file header
    if (avformat_write_header(o_pFormatCtx, NULL) < 0) {  //写文件头（对于某些没有文件头的封装格式，不需要此函数。比如说MPEG2TS）。
        printf( "Error occurred when opening output file\n");
        //goto end;
    }

    qDebug() << "initial output successfully";
    return true;
}

void ffmpegDecode::setRecordState(bool state)
{
    isRecord = state;
}

void ffmpegDecode::h264Decodec()
{
    //一帧一帧读取视频
    int frameFinished=0;
    while(av_read_frame(pFormatCtx, &packet) >= 0 && flag){
		if(packet.stream_index==videoStreamIndex)
		{
			//qDebug()<<"开始解码"<<QDateTime::currentDateTime().toString("HH:mm:ss zzz");
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			if (frameFinished)
			{
				printf("***************ffmpeg decodec*******************\n");
				mutex.lock();

                if(isRecord)
                {
                    //output
                    AVStream *in_stream, *out_stream;
                    in_stream  = pFormatCtx->streams[packet.stream_index];
                    out_stream = o_pFormatCtx->streams[packet.stream_index];
                    //Convert PTS/DTS
                    packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                    packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
                    packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
                    packet.pos = -1;
                    //Write
                    if (av_interleaved_write_frame(o_pFormatCtx, &packet) < 0) { //将AVPacket（存储视频压缩码流数据）写入文件。
                        printf( "Error muxing packet\n");
                        break;
                    }
                }

				int rs = sws_scale(pSwsCtx, (const uint8_t* const *) pFrame->data,
					pFrame->linesize, 0,
					height, pAVPicture.data, pAVPicture.linesize);

				//发送获取一帧图像信号
				QImage image(pAVPicture.data[0],width,height,QImage::Format_RGB888);
				emit GetImage(image);

				mutex.unlock();
			}
		}
        av_free_packet(&packet);//释放资源,否则内存会一直上升
    }
    if(isRecord)
    {
        wFileTrailer();
    }
}

void ffmpegDecode::wFileTrailer()
{
    //Write file trailer
    av_write_trailer(o_pFormatCtx);//写文件尾（对于某些没有文件头的封装格式，不需要此函数。比如说MPEG2TS）。\
    ///* close output */
    if (o_pFormatCtx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(o_pFormatCtx->pb);
}

void ffmpegDecode::delRecord()
{
    avformat_free_context(o_pFormatCtx);
    //avformat_free_context(ofmt);

}
