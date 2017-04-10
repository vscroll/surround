#ifndef CAPTURE4WORKERV4L2IMPL_H
#define CAPTURE4WORKERV4L2IMPL_H

#include <QQueue>
#include <QMutex>

#include "common.h"
#include "capture4workerbase.h"
#include "v4l2.h"

class Capture4WorkerV4l2Impl : public Capture4WorkerBase
{
    Q_OBJECT
public:
    explicit Capture4WorkerV4l2Impl(QObject *parent = 0, int videoChannelNum = 4);

public:
    virtual int openDevice();
    virtual void closeDevice();
signals:

public slots:
    virtual void onCapture();
private:
    int mWidth[VIDEO_CHANNEL_SIZE];
    int mHeight[VIDEO_CHANNEL_SIZE];
    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[VIDEO_CHANNEL_SIZE][V4l2::V4L2_BUF_COUNT];
    int mIPUFd[VIDEO_CHANNEL_SIZE];
    int mVideoFd[VIDEO_CHANNEL_SIZE];
    QMutex mMutexCapture;

    struct V4l2::buffer mIpuBuf[VIDEO_CHANNEL_SIZE][V4l2::V4L2_BUF_COUNT];
};

#endif // CAPTURE4WORKERV4L2IMPL_H
