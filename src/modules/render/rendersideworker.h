#ifndef RENDERSIDEWORKER_H
#define RENDERSIDEWORKER_H

#include "renderbase.h"
#include "wrap_thread.h"

class ICapture;
class ImageSHM;
class RenderSideWorker : public RenderBase, public WrapThread
{
public:
    RenderSideWorker();
    virtual ~RenderSideWorker();

    void setCaptureModule(ICapture* capture);
    void setSideImageCrop(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height);
    void setSideImageRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height);
    void getSideImageRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height);

public:
    virtual void run();

private:
    ICapture* mCapture;
    ImageSHM* mImageSHM;

    unsigned int mSideImageCropLeft;
    unsigned int mSideImageCropTop;
    unsigned int mSideImageCropWidth;
    unsigned int mSideImageCropHeight;

    unsigned int mSideImageLeft;
    unsigned int mSideImageTop;
    unsigned int mSideImageWidth;
    unsigned int mSideImageHeight;

    unsigned int mFocusChannelIndex;

    clock_t mLastCallTime;
};

#endif // RENDERSIDEWORKER_H
