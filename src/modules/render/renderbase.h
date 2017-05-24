#ifndef RENDERBASE_H
#define RENDERBASE_H

#include "renderdevice.h"

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
    RenderDevice* mRenderDevice;
};

#endif // RENDERBASE_H
