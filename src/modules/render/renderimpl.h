#ifndef RENDERIMPL_H
#define RENDERIMPL_H

#include "IRender.h"

#define RENDER_MARK_ALONE 0

class RenderSideWorker;
class RenderMarkWorker;
class RenderPanoWorker;
class RenderImpl : public IRender
{
public:
    RenderImpl();
    virtual ~RenderImpl();

    virtual void setCaptureModule(ICapture* capture = NULL);
    virtual void setSideImageRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height);
    virtual void setChannelMarkRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height);

    virtual void setPanoImageModule(IPanoImage* panoImage = NULL);
    virtual void setPanoImageRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height);

    virtual int start(unsigned int fps);
    virtual void stop();
private:
    RenderSideWorker* mSideWorker;
#if RENDER_MARK_ALONE
    RenderMarkWorker* mMarkWorker;
#endif
    RenderPanoWorker* mPanoWorker;
};

#endif // RENDERIMPL_H
