#include "renderbase.h"
#include <string.h>
#include <iostream>
#include "renderdevice.h"

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

void RenderBase::drawImage(unsigned char* buf,
            unsigned int srcPixfmt,
            unsigned int srcWidth,
            unsigned int srcHeight,
            unsigned int srcSize)
{
    mRenderDevice->drawImage(buf,
                srcPixfmt,
                srcWidth,
                srcHeight,
                srcSize,
                mRenderDevice->getDstLeft(),
                mRenderDevice->getDstTop(),
                mRenderDevice->getDstWidth(),
                mRenderDevice->getDstHeight());
}
