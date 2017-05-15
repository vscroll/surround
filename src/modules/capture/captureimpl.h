#ifndef CAPTUREIMPL_H
#define CAPTUREIMPL_H

#include "ICapture.h"
#include "captureworkerbase.h"

class CaptureWorkerBase;
class CaptureImpl : public ICapture
{
public:
    CaptureImpl();
    virtual ~CaptureImpl();

    virtual void setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum);
    virtual void setFocusSource(int focusChannelIndex, struct cap_src_t* focusSource);
    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual void closeDevice();
    virtual int start(unsigned int fps);
    virtual void stop();
    virtual int getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual int getFPS(unsigned int* fps);
    virtual surround_images_t* popOneFrame();
    virtual surround_image_t* popOneFrame4FocusSource();

private:
    CaptureWorkerBase *mCaptureWorker;
};

#endif // CAPTUREIMPL_H
