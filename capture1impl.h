#ifndef CAPTURE1IMPL_H
#define CAPTURE1IMPL_H

#include <QObject>
#include <QTimer>
#include <QThread>

#include "ICapture.h"

class Capture1WorkerBase;
class Capture1Impl : public QObject, public ICapture
{
    Q_OBJECT
public:
    explicit Capture1Impl(QObject *parent = 0);

    virtual int openDevice();
    virtual int closeDevice();
    virtual int start(VIDEO_FPS fps);
    virtual int stop();
    virtual surround_images_t* popOneFrame();
signals:

public slots:

private:
    QTimer mVideoCaptureTimer[VIDEO_CHANNEL_SIZE];
    Capture1WorkerBase *mCaptureWorker[VIDEO_CHANNEL_SIZE];
    QThread mCaptureThread[VIDEO_CHANNEL_SIZE];
    VIDEO_FPS mFPS;
};

#endif // CAPTURE1IMPL_H
