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

    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual int closeDevice();
    virtual int start(unsigned int fps);
    virtual int stop();
    virtual surround_images_t* popOneFrame();

private:
    Capture4WorkerBase *mCaptureWorker;
};

#endif // CAPTURE4IMPL_H
