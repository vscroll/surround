#ifndef CAPTURE1IMPL_H
#define CAPTURE1IMPL_H

#include "ICapture.h"

class Capture1WorkerV4l2;
class Capture1Impl : public ICapture
{
public:
    Capture1Impl(unsigned int channelNum);
    virtual ~Capture1Impl();

    virtual void setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum);
    virtual void setFocusSource(unsigned int focusChannelIndex, struct cap_src_t* focusSource);
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
    unsigned int mChannelNum;
    unsigned int mFocusChannelIndex;
    Capture1WorkerV4l2* mCaptureWorker[VIDEO_CHANNEL_SIZE];
};

#endif // CAPTURE1IMPL_H
