#ifndef CAPTUREIMPL_H
#define CAPTUREIMPL_H

#include "ICapture.h"

class CaptureWorkerV4l2;
class CaptureImpl : public ICapture
{
public:
    CaptureImpl();
    virtual ~CaptureImpl();

    virtual int setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum);
    virtual int setFocusSource(unsigned int focusChannelIndex, struct cap_src_t* focusSource);
    virtual unsigned int getFocusChannelIndex();
    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual void closeDevice();
    virtual int start(unsigned int fps);
    virtual void stop();
    virtual int getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual int getFPS(unsigned int channelIndex, unsigned int* fps);
    virtual surround_images_t* popOneFrame();
    virtual surround_image_t* popOneFrame4FocusSource();
    virtual void enableCapture();
    virtual surround_image_t* captureOneFrame4FocusSource();

    virtual surround_image_t* popOneFrame(unsigned int channelIndex);
private:
    CaptureWorkerV4l2 *mCaptureWorker;
};

#endif // CAPTUREIMPL_H
