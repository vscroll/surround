#ifndef CAPTURE1WORKERV4L2_H
#define CAPTURE1WORKERV4L2_H

#include "common.h"
#include "capture1workerbase.h"
#include "v4l2.h"
#include "imxipu.h"
#include "wrap_thread.h"

class Capture1WorkerV4l2 : public Capture1WorkerBase, public WrapThread
{
public:
    Capture1WorkerV4l2();
    virtual ~Capture1WorkerV4l2();

    virtual int openDevice(unsigned int channel);
    virtual void closeDevice();

    virtual void run();

private:
    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[V4L2_BUF_COUNT];

    int mIPUFd;
    struct IMXIPU::buffer mInIPUBuf;
    struct IMXIPU::buffer mOutIPUBuf;

    pthread_mutex_t mMutexV4l2;
    pthread_mutex_t mMutexIpu;
};

#endif // CAPTURE1WORKERV4L2_H
