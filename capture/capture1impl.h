#ifndef CAPTURE1IMPL_H
#define CAPTURE1IMPL_H

#include "ICapture.h"

class Capture1WorkerBase;
class Capture1Impl : public ICapture
{
public:
    Capture1Impl();
    virtual ~Capture1Impl();

    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual int closeDevice();
    virtual int start(unsigned int fps);
    virtual int stop();
    virtual surround_images_t* popOneFrame();

private:
    unsigned int mChannelNum;
    Capture1WorkerBase *mCaptureWorker[VIDEO_CHANNEL_SIZE];
};

#endif // CAPTURE1IMPL_H
