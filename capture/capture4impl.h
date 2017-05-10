#ifndef CAPTURE4IMPL_H
#define CAPTURE4IMPL_H

#include "ICapture.h"
#include "capture4workerbase.h"

class Capture4WorkerBase;
class Capture4Impl : public ICapture
{
public:
    Capture4Impl();
    virtual ~Capture4Impl();

    virtual void setCapCapacity(struct cap_sink_t sink[], struct cap_src_t sideSrc[], struct cap_src_t panoSrc[], unsigned int channelNum);
    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual int closeDevice();
    virtual int start(unsigned int fps);
    virtual int stop();
    virtual void getSideResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual void getPanoResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual int getFPS(unsigned int* fps);
    virtual surround_images_t* popOneFrame();

private:
    Capture4WorkerBase *mCaptureWorker;
};

#endif // CAPTURE4IMPL_H
