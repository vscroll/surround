#ifndef RENDERBASE_H
#define RENDERBASE_H

#include "renderdevice.h"
#include <time.h>

class RenderBase
{
public:
    RenderBase();
    virtual ~RenderBase();
    virtual int openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight);

    virtual void closeDevice();

    virtual void drawImage(struct render_surface_t* surface, bool alpha = false);

    virtual void drawMultiImages(struct render_surface_t surfaces[], unsigned int num, bool alpha = false);

protected:
    unsigned int statFPS();

protected:
    RenderDevice* mRenderDevice;

    unsigned int mRealFPS;
    clock_t mStartStatTime;
    clock_t mStatDuration;
    unsigned long mRealFrameCount;
};

#endif // RENDERBASE_H
