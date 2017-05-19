#include "renderbase.h"
#include <string.h>
#include <iostream>

RenderBase::RenderBase()
{
    mRenderDevice = new RenderDevice();
}

RenderBase::~RenderBase()
{

}

int RenderBase::openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight)
{
    if (mRenderDevice->openDevice(dstLeft, dstTop, dstWidth, dstHeight) < 0)
    {
	    return -1;
    }    

    return 0;
}

void RenderBase::closeDevice()
{
    mRenderDevice->closeDevice();
}

void RenderBase::drawImage(struct render_surface_t* surface, bool alpha)
{
    mRenderDevice->drawImage(surface, alpha);
}

void RenderBase::drawMultiImages(struct render_surface_t surfaces[], unsigned int num, bool alpha)
{
    mRenderDevice->drawMultiImages(surfaces, num, alpha);
}
