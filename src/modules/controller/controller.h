#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "thread.h"

class ICapture;
class IPanoImage;
class ISideImage;
class ImageSHM;
class PanoSHMWorker;
class IRender;
class Controller : public Thread
{
public:
    Controller();
    virtual ~Controller();

    ICapture* initCaptureModule(
            unsigned int channel[],
            unsigned int channelNum,
            struct cap_sink_t sink[],
            struct cap_src_t source[]);

    IPanoImage* initPanoImageModule(
            ICapture* capture,
            unsigned int inWidth,
            unsigned int inHeight,
            unsigned int inPixfmt,
            unsigned int panoWidth,
            unsigned int panoHeight,
            unsigned int panoPixfmt,
            char* algoCfgFilePath,
            bool enableOpenCL);

    ISideImage* initSideImageModule(
            ICapture* capture,
            unsigned int focusChannelIndex,
            unsigned int outWidth,
            unsigned int outHeight,
            unsigned int outPixfmt);

    IRender* initRenderModule(
            ICapture* capture,
            ISideImage* sideImage,
            IPanoImage* panoImage,
		    unsigned int sideLeft,
		    unsigned int sideTop,
		    unsigned int sideWidth,
		    unsigned int sideHeight,
		    unsigned int panoLeft,
		    unsigned int panoTop,
		    unsigned int panoWidth,
		    unsigned int panoHeight);    

    void uninitModules();
    void startModules(unsigned int fps);
    void stopModules();

    void startLoop(unsigned int freq);
    void stopLoop();
public:
    virtual void run();

private:
    ICapture* mCapture;
    IPanoImage* mPanoImage;
    IRender* mRender;

    unsigned int mFocusChannelIndex;

    ImageSHM* mSideSHM;
    ImageSHM* mPanoSHM;

    PanoSHMWorker* mPanoSHMWorker;
};

#endif // CONTROLLER_H
