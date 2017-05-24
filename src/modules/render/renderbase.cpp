#include "renderbase.h"
#include <string.h>
#include <iostream>

RenderBase::RenderBase()
{
    mRenderDevice = NULL;
}

RenderBase::~RenderBase()
{

}

int RenderBase::openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight)
{
    if (NULL == mRenderDevice)
    {
        mRenderDevice = new RenderDevice(0, true);
    }

    if (mRenderDevice->openDevice(dstLeft, dstTop, dstWidth, dstHeight) < 0)
    {
	    return -1;
    }    

    return 0;
}

void RenderBase::closeDevice()
{
    if (NULL != mRenderDevice)
    {
        mRenderDevice->closeDevice();
        delete mRenderDevice;
        mRenderDevice = NULL;
    }
}

void RenderBase::drawImage(struct render_surface_t* surface, bool alpha)
{
    if (NULL != mRenderDevice)
    {
        mRenderDevice->drawImage(surface, alpha);
    }
}

void RenderBase::drawMultiImages(struct render_surface_t surfaces[], unsigned int num, bool alpha)
{
    if (NULL != mRenderDevice)
    {
        mRenderDevice->drawMultiImages(surfaces, num, alpha);
    }
}
