#ifndef RENDERIMPL_H
#define RENDERIMPL_H

#include "IRender.h"

class RenderSideWorker;
class RenderPanoWorker;
class RenderImpl : public IRender
{
public:
    RenderImpl();
    virtual ~RenderImpl();

    virtual int init(
        ICapture* capture,
        IPanoImage* panoImage,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight,
		unsigned int panoLeft,
		unsigned int panoTop,
		unsigned int panoWidth,
		unsigned int panoHeight);
    virtual int start(unsigned int fps);
    virtual void stop();
private:
    RenderSideWorker* mSideWorker;
    RenderPanoWorker* mPanoWorker;
};

#endif // RENDERIMPL_H
