#ifndef ISTITCH_H
#define ISTITCH_H

#include "common.h"

class IStitch
{
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void append(surround_image4_t* images) = 0;
    virtual surround_image1_t* dequeueFullImage() = 0;
    virtual surround_image1_t* dequeueSmallImage(VIDEO_CHANNEL channel) = 0;
};

#endif // ISTITCH_H
