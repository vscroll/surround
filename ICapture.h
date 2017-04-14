#ifndef ICAPTURE_H
#define ICAPTURE_H

#include "common.h"

class ICapture
{
public:
    virtual ~ICapture() {}
    virtual int openDevice() = 0;
    virtual int closeDevice() = 0;
    virtual int start(int fps) = 0;
    virtual int stop() = 0;
    virtual surround_images_t* popOneFrame() = 0;
};

#endif // ICAPTURE_H
