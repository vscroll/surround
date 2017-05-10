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
    virtual int openDevice(unsigned int channel[], struct cap_info_t capInfo[], unsigned int channelNum);
    virtual void closeDevice();
    virtual void getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual int getFPS(unsigned int* fps);
    virtual void run();

private:
    struct cap_info_t mCapInfo[VIDEO_CHANNEL_SIZE];
    unsigned mInFrameSize;
    unsigned mOutFrameSize;

    int mVideoFd[VIDEO_CHANNEL_SIZE];
    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[VIDEO_CHANNEL_SIZE][V4L2_BUF_COUNT];

    int mIPUFd[VIDEO_CHANNEL_SIZE];
    struct IMXIPU::buffer mInIPUBuf[VIDEO_CHANNEL_SIZE];
    struct IMXIPU::buffer mOutIPUBuf[VIDEO_CHANNEL_SIZE];

    unsigned int mRealFPS;
    double mStartTime;
    double mStatDuration;
    unsigned long mRealFrameCount;
};

#endif // CAPTURE4WORKERV4L2IMPL_H
