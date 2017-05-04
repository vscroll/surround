#ifndef CAPTURE4WORKERV4L2IMPL_H
#define CAPTURE4WORKERV4L2IMPL_H

#include "common.h"
#include "capture4workerbase.h"
#include "v4l2.h"
#include "imxipu.h"

class Capture4WorkerV4l2Impl : public Capture4WorkerBase
{
public:
    Capture4WorkerV4l2Impl();
    virtual ~Capture4WorkerV4l2Impl();

public:
    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual void closeDevice();
    virtual void getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual void run();

private:
    unsigned int mInWidth[VIDEO_CHANNEL_SIZE];
    unsigned int mInHeight[VIDEO_CHANNEL_SIZE];
    unsigned int mInPixfmt[VIDEO_CHANNEL_SIZE];
    unsigned int mOutWidth[VIDEO_CHANNEL_SIZE];
    unsigned int mOutHeight[VIDEO_CHANNEL_SIZE];
    unsigned int mOutPixfmt[VIDEO_CHANNEL_SIZE];

    int mVideoFd[VIDEO_CHANNEL_SIZE];
    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[VIDEO_CHANNEL_SIZE][V4L2_BUF_COUNT];

    int mIPUFd[VIDEO_CHANNEL_SIZE];
    struct IMXIPU::buffer mInIPUBuf[VIDEO_CHANNEL_SIZE];
    struct IMXIPU::buffer mOutIPUBuf[VIDEO_CHANNEL_SIZE];
};

#endif // CAPTURE4WORKERV4L2IMPL_H
