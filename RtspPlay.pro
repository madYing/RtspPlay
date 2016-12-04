#-------------------------------------------------
#
# Project created by QtCreator 2016-10-28T09:54:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RtspPlay
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ffmpegdecode.cpp \
    rtspthread.cpp

HEADERS  += mainwindow.h \
    ffmpegdecode.h \
    rtspthread.h

FORMS    += mainwindow.ui

INCLUDEPATH +=ffmpeg\
            ffmpeg/Include \

LIBS += ffmpeg/lib/avcodec.lib \
    -lffmpeg/lib/avfilter \
    -lffmpeg/lib/avformat \
    -lffmpeg/lib/swscale \
    -lffmpeg/lib/avutil
