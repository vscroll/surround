#ifndef STITCHIMPL_H
#define STITCHIMPL_H

#include "IStitch.h"

class StitchWorker;
class StitchImpl : public IStitch
{
public:
    StitchImpl();
    virtual ~StitchImpl();

    virtual void start(ICapture *capture,
		unsigned int fps,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		char* configFilePath,
		bool enableOpenCL);
    virtual void stop();
    virtual surround_image_t* dequeuePano2DImage();
    virtual surround_image_t* dequeueSideImage(unsigned int channelIndex);

private:
    StitchWorker *mWorker;
};

#endif // STITCHIMPL_H
