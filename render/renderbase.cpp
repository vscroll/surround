#include "renderbase.h"
#include <string.h>
#include <iostream>
#include "renderdevice.h"

RenderBase::RenderBase()
{
    mStitch = NULL;
    mRenderDevice = new RenderDevice();
}

RenderBase::~RenderBase()
{

}

int RenderBase::init(IStitch *stitch,
		unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height)
{
    mStitch = stitch;
    if (mRenderDevice->openDevice(left, top, width, height) < 0)
    {
	return -1;
    }    

    return 0;
}

void RenderBase::uninit()
{
    mRenderDevice->closeDevice();
}

void RenderBase::drawImage(unsigned char* buf, unsigned int size)
{
    mRenderDevice->drawImage(buf, size);
}
