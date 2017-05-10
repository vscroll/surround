#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"

class ICapture;
class IStitch;
class IRender;
class Controller
{
public:
    Controller();
    virtual ~Controller();

    void init(unsigned int channel[], unsigned int channelNum);
    void uninit();
    void start(unsigned int fps,
		unsigned int pano2DLeft,
		unsigned int pano2DTop,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight,
		char* configFilePath,
		bool enableOpenCL);
    void stop();
    surround_image_t* dequeuePano2DImage();
    surround_image_t* dequeueSideImage(unsigned int channelIndex);

private:
    ICapture* mCapture;
    IStitch* mStitch;
    IRender* mRender;
};

#endif // CONTROLLER_H
