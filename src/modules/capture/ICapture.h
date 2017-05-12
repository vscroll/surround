#ifndef ICAPTURE_H
#define ICAPTURE_H

#include "common.h"

class ICapture
{
public:
    virtual ~ICapture() {}
    virtual void setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum) = 0;
    virtual int openDevice(unsigned int channel[], unsigned int channelNum) = 0;
    virtual void closeDevice() = 0;
    virtual int start(unsigned int fps) = 0;
    virtual void stop() = 0;
    virtual int getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height) = 0;
    virtual int getFPS(unsigned int* fps) = 0;
    virtual surround_images_t* popOneFrame() = 0;
};

#endif // ICAPTURE_H
