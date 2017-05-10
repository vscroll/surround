#ifndef RENDERIMPL_H
#define RENDERIMPL_H

#include "IRender.h"

class RenderPano2DWorker;
class RenderImpl : public IRender
{
public:
    RenderImpl();
    virtual ~RenderImpl();

    virtual int start(IStitch *stitch,
                unsigned int fps,
		unsigned int pano2DLeft,
		unsigned int pano2DTop,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight);
    virtual void stop();

private:
    RenderPano2DWorker *mWorker;
};

#endif // RENDERIMPL_H
