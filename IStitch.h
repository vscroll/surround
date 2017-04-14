#ifndef ISTITCH_H
#define ISTITCH_H

#include "common.h"

class ICapture;
class IStitch
{
public:
    virtual ~IStitch() {}
    virtual void start(ICapture* capture) = 0;
    virtual void stop() = 0;
    virtual surround_image_t* dequeueFullImage() = 0;
    virtual surround_image_t* dequeueSmallImage(VIDEO_CHANNEL channel) = 0;
};

#endif // ISTITCH_H
