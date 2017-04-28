#ifndef CAPTURE1WORKERV4L2IMPL_H
#define CAPTURE1WORKERV4L2IMPL_H

#include "common.h"
#include "capture1workerbase.h"
#include "v4l2.h"
#include "imxipu.h"
#include <pthread.h>

class Capture1WorkerV4l2Impl : public Capture1WorkerBase
{
public:
    Capture1WorkerV4l2Impl();
    virtual ~Capture1WorkerV4l2Impl();

    virtual int openDevice(unsigned int channel);
    virtual void closeDevice();
    virtual void onCapture();

private:
    unsigned int mInWidth;
    unsigned int mInHeight;
    unsigned int mInPixfmt;
    unsigned int mOutWidth;
    unsigned int mOutHeight;
    unsigned int mOutPixfmt;

    int mIPUFd;

    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[V4L2_BUF_COUNT];
    int mVideoFd;

    struct IMXIPU::buffer mInIPUBuf;
    struct IMXIPU::buffer mOutIPUBuf;

    pthread_mutex_t mMutexV4l2;
    pthread_mutex_t mMutexIpu;
};

#endif // CAPTURE1WORKERV4L2IMPL_H
