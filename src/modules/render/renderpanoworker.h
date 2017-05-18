#ifndef RENDERPANOWORKER_H
#define RENDERPANOWORKER_H

#include "renderbase.h"
#include "thread.h"

class IPanoImage;
class ImageSHM;
class RenderPanoWorker : public RenderBase, public Thread
{
public:
    RenderPanoWorker();
    virtual ~RenderPanoWorker();

    void setPanoImageModule(IPanoImage* panoImage);
    void setPanoImageRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height);
    void getPanoImageRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height);
public:
    virtual void run();

private:
    IPanoImage* mPanoImage;
    ImageSHM* mPanoSHM;

    unsigned int mPanoImageLeft;
    unsigned int mPanoImageTop;
    unsigned int mPanoImageWidth;
    unsigned int mPanoImageHeight;

    clock_t mLastCallTime;
};

#endif // RENDERPANOWORKER_H
