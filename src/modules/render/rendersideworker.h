#ifndef RENDERSIDEWORKER_H
#define RENDERSIDEWORKER_H

#include "renderbase.h"
#include "thread.h"

class ICapture;
class ImageSHM;
class RenderSideWorker : public RenderBase, public Thread
{
public:
    RenderSideWorker();
    virtual ~RenderSideWorker();

    void setCaptureModule(ICapture* capture);
    void setSideImageRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height);
    void getSideImageRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height);
    void setChannelMarkRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height);
public:
    virtual void run();

private:
    ICapture* mCapture;
    ImageSHM* mImageSHM;

    unsigned int mSideImageLeft;
    unsigned int mSideImageTop;
    unsigned int mSideImageWidth;
    unsigned int mSideImageHeight;

    unsigned int mChannelMarkLeft;
    unsigned int mChannelMarkTop;
    unsigned int mChannelMarkWidth;
    unsigned int mChannelMarkHeight;

    unsigned int mFocusChannelIndex;

    clock_t mLastCallTime;
};

#endif // RENDERSIDEWORKER_H
