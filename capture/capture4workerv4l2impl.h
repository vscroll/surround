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
    virtual void setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum);
    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual void closeDevice();
    virtual void getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual int getFPS(unsigned int* fps);
    virtual void run();

private:
    struct cap_sink_t mSink[VIDEO_CHANNEL_SIZE];
    struct cap_src_t mSource[VIDEO_CHANNEL_SIZE];
    unsigned int mSinkFrameSize[VIDEO_CHANNEL_SIZE];
    unsigned int mSourceFrameSize[VIDEO_CHANNEL_SIZE];

    int mVideoFd[VIDEO_CHANNEL_SIZE];
    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[VIDEO_CHANNEL_SIZE][V4L2_BUF_COUNT];

    int mIPUFd;
    struct IMXIPU::buffer mInIPUBuf[VIDEO_CHANNEL_SIZE];
    struct IMXIPU::buffer mOutIPUBuf[VIDEO_CHANNEL_SIZE];

    unsigned int mRealFPS;
    double mStartTime;
    double mStatDuration;
    unsigned long mRealFrameCount;
};

#endif // CAPTURE4WORKERV4L2IMPL_H
