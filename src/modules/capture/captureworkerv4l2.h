#ifndef CAPTUREWORKERV4L2_H
#define CAPTUREWORKERV4L2_H

#include "common.h"
#include "captureworkerbase.h"
#include "v4l2.h"
#include "imxipu.h"

class CaptureWorkerV4l2 : public CaptureWorkerBase
{
public:
    CaptureWorkerV4l2();
    virtual ~CaptureWorkerV4l2();

public:
    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual void closeDevice();
    virtual void run();

private:
    void clearOverstock();
    bool isNeedConvert(unsigned int channelIndex);
private:
    v4l2_memory mMemType;
    struct V4l2::buffer mV4l2Buf[VIDEO_CHANNEL_SIZE][V4L2_BUF_COUNT];

    int mIPUFd;
    struct IMXIPU::buffer mInIPUBuf[VIDEO_CHANNEL_SIZE];
    struct IMXIPU::buffer mOutIPUBuf[VIDEO_CHANNEL_SIZE];
};

#endif // CAPTUREWORKERV4L2_H
