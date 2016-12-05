#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->hide();
    ui->recordBut->setText(QString::fromLocal8Bit("录制"));
    recordFlag = true;//true表示可录制，false表示可停止
    ffmpeg = NULL;
    rtspthread = NULL;
}

MainWindow::~MainWindow()
{
    if(ffmpeg)
    {
        disconnect(ffmpeg,SIGNAL(GetImage(QImage)),this,SLOT(SetImage(QImage)));
        ffmpeg->flag = false;
       // ffmpeg->deleteLater();
        //rtspthread->deleteLater();
    }
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    if(!videoimage.isNull())
    {
        QPainter painter(this);
       // QPixmap  pix =  QPixmap::fromImage(videoimage.scaled(ui->widget->width(),ui->widget->height(),Qt::KeepAspectRatio));
        double widget_W,widget_h;
        widget_h = ui->widget->height();
        widget_W = ui->widget->width();
        QImage temp = videoimage.scaled(ui->widget->width(),ui->widget->height(),Qt::KeepAspectRatio);
        int offset_x,offset_y;
        offset_x = (widget_W - temp.width()) /2;
        offset_y = (widget_h - temp.height()) /2;
        painter.translate(offset_x,offset_y);
        painter.drawImage(9,9,temp);
        //painter.drawImage(ui->widget->rect(),videoimage.scaled(ui->widget->width(),ui->widget->height(),Qt::KeepAspectRatio));
//        painter.drawPixmap(ui->widget->rect(),pix);
    }
}

void MainWindow::on_playBtn_clicked()
{
    //ffmpegDecode *ffmpeg = new ffmpegDecode(this);
    ffmpeg = new ffmpegDecode(this);
    connect(ffmpeg,SIGNAL(GetImage(QImage)),this,SLOT(SetImage(QImage)));
    ffmpeg->SetUrl(ui->lineEdit->text());
    qDebug() << "url : " << ui->lineEdit->text();

    if(ffmpeg->init())
    {
        //RtspThread *rtspthread = new RtspThread();
        rtspthread = new RtspThread();
        rtspthread->setFFmpeg(ffmpeg);
        rtspthread->start();
    }else
    {
        QMessageBox::warning(this,QString::fromLocal8Bit("警告"),QString::fromLocal8Bit("初始化解码器错误！"),QMessageBox::Ok);
    }
}

void MainWindow::SetImage(const QImage &image)
{
    if(image.height()>0)
    {
        //ui->video_label->resize(image.width(),image.height());
       // QPixmap  pix =  QPixmap::fromImage(image.scaled(ui->video_label->width(),ui->video_label->height(),Qt::KeepAspectRatio));
        videoimage = image;
        update();
        //QPixmap pix = QPixmap::fromImage(image);
//        ui->video_label->setAlignment(Qt::AlignCenter);
//        ui->video_label->setPixmap(pix);
    }
}

void MainWindow::on_recordBut_clicked()
{
    if(recordFlag)
    {
        bool flag = ffmpeg->initRecord();
        ffmpeg->setRecordState(true);
		recordFlag = false;
        ui->recordBut->setText(QString::fromLocal8Bit("停止"));
    }else
    {
		ffmpeg->setRecordState(false);
        ffmpeg->wFileTrailer();
        ffmpeg->delRecord();
		recordFlag = true;
        ui->recordBut->setText(QString::fromLocal8Bit("录制"));
    }
}
