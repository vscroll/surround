#include "rendermarkworker.h"
#include <string.h>
#include <iostream>
#include "common.h"
#include "util.h"
#include "renderdevice.h"

RenderMarkWorker::RenderMarkWorker()
{
    mMarkLeft = 0;
    mMarkTop = 0;
    mMarkWidth = 0;
    mMarkHeight = 0;

    mFocusChannelIndex = VIDEO_CHANNEL_SIZE;
    mUpdateChannelIndex = VIDEO_CHANNEL_FRONT;

    mLastCallTime = 0;
}

RenderMarkWorker::~RenderMarkWorker()
{
}

void RenderMarkWorker::setMarkRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height)
{
    mMarkLeft = left;
    mMarkTop = top;
    mMarkWidth = width;
    mMarkHeight = height;
}

void RenderMarkWorker::getMarkRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height)
{
    *left = mMarkLeft;
    *top = mMarkTop;
    *width = mMarkWidth;
    *height = mMarkHeight;
}

int RenderMarkWorker::openDevice(unsigned int dstLeft,
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

unsigned char gMarkData[100*100*2] = {0};
void RenderMarkWorker::run()
{
#if DEBUG_UPDATE
    clock_t start = clock();
    double elapsed_to_last = 0;
    if (mLastCallTime != 0)
    {
        elapsed_to_last = (double)(start - mLastCallTime)/CLOCKS_PER_SEC;
    }
    mLastCallTime = start;
#endif


#if DEBUG_UPDATE
    clock_t start_draw = clock();
#endif

    // (mFocusChannelIndex != mUpdateChannelIndex)
    {
        mFocusChannelIndex = mUpdateChannelIndex;
        struct render_surface_t surface;
        surface.srcBuf = (unsigned char*)gMarkData;
        surface.srcPixfmt = V4L2_PIX_FMT_YUYV;
        surface.srcLeft = 0;
        surface.srcTop = 0;
		surface.srcStride = 100;
        surface.srcWidth = 100;
        surface.srcHeight = 100;
        surface.srcSize = surface.srcWidth*surface.srcHeight*2;
        surface.dstLeft = mMarkLeft;
        surface.dstTop = mMarkTop;
        surface.dstWidth = mMarkWidth;
        surface.dstHeight = mMarkHeight;
        drawImage(&surface);
    }

#if DEBUG_UPDATE
    std::cout << "RenderMarkWorker::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed_to_last
             << ", draw:" << (double)(clock() - start_draw)/CLOCKS_PER_SEC
            << std::endl;
#endif
}

