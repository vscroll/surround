#ifndef ISTITCH_H
#define ISTITCH_H

#include "common.h"

class ICapture;
class IStitch
{
public:
    virtual ~IStitch() {}
    virtual void start(ICapture *capture,
		unsigned int fps,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		char* configFilePath,
		bool enableOpenCL) = 0;
    virtual void stop() = 0;
    virtual surround_image_t* dequeuePano2DImage() = 0;
    virtual surround_image_t* dequeueSideImage(unsigned int channelIndex) = 0;
};

#endif // ISTITCH_H
