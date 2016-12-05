#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>

class videoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit videoWidget(QWidget *parent = 0);
    ~videoWidget();

protected:
    void paintEvent(QPaintEvent *);

signals:

public slots:
};

#endif // VIDEOWIDGET_H
