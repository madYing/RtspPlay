#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ffmpegdecode.h"
#include <QImage>
#include "rtspthread.h"
#include "videowidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void paintEvent(QPaintEvent *event);

private slots:
    void on_playBtn_clicked();
    void SetImage(const QImage &image);

    void on_recordBut_clicked();

private:
    Ui::MainWindow *ui;
    bool recordFlag;
    ffmpegDecode *ffmpeg;
    RtspThread *rtspthread;
    QImage videoimage;

	videoWidget *videView;
};

#endif // MAINWINDOW_H
