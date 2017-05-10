#ifndef ICAPTURE_H
#define ICAPTURE_H

#include "common.h"

class ICapture
{
public:
    virtual ~ICapture() {}
    virtual int openDevice(unsigned int channel[], struct cap_info_t capInfo[], unsigned int channelNum) = 0;
    virtual int closeDevice() = 0;
    virtual int start(unsigned int fps) = 0;
    virtual int stop() = 0;
    virtual void getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height) = 0;
    virtual int getFPS(unsigned int* fps) = 0;
    virtual surround_images_t* popOneFrame() = 0;
};

#endif // ICAPTURE_H
