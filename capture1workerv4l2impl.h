#ifndef CAPTURE1WORKERV4L2IMPL_H
#define CAPTURE1WORKERV4L2IMPL_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include "common.h"
#include "capture1workerbase.h"
#include "v4l2.h"

class Capture1WorkerV4l2Impl : public Capture1WorkerBase
{
    Q_OBJECT
public:
    explicit Capture1WorkerV4l2Impl(QObject *parent = 0, int videoChannel = 0);

    virtual int openDevice();
    virtual void closeDevice();
signals:

public slots:
    virtual void onCapture();

private:
    int mWidth;
    int mHeight;
    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[V4L2_BUF_COUNT];
    int mVideoFd;
    int mIPUFd;
    struct V4l2::buffer mIpuBuf;
    QMutex mMutexV4l2;
    QMutex mMutexIpu;

    int mDropFrameCount;
    QMutex mMutexDrop;
};

#endif // CAPTURE1WORKERV4L2IMPL_H
