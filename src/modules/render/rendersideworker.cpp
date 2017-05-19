#include "rendersideworker.h"
#include <string.h>
#include <iostream>
#include "common.h"
#include "ICapture.h"
#include "util.h"
#include "imageshm.h"
#include "renderdevice.h"

RenderSideWorker::RenderSideWorker()
{
    mCapture = NULL;
    mSideSHM = NULL;

    mSideImageLeft = 0;
    mSideImageTop = 0;
    mSideImageWidth = 0;
    mSideImageHeight = 0;

    mChannelMarkLeft = 0;
    mChannelMarkTop = 0;
    mChannelMarkWidth = 0;
    mChannelMarkHeight = 0;

    mFocusChannelIndex = VIDEO_CHANNEL_FRONT;

    mLastCallTime = 0;
}

RenderSideWorker::~RenderSideWorker()
{
}

void RenderSideWorker::setCaptureModule(ICapture* capture)
{
    mCapture = capture;
    if (NULL == capture)
    {
        mSideSHM = new ImageSHM();
        mSideSHM->create((key_t)SHM_SIDE_ID, SHM_SIDE_SIZE);
    }
}

void RenderSideWorker::setSideImageRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height)
{
    mSideImageLeft = left;
    mSideImageTop = top;
    mSideImageWidth = width;
    mSideImageHeight = height;
}

void RenderSideWorker::getSideImageRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height)
{
    *left = mSideImageLeft;
    *top = mSideImageTop;
    *width = mSideImageWidth;
    *height = mSideImageHeight;
}

void RenderSideWorker::setChannelMarkRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height)
{
    mChannelMarkLeft = left;
    mChannelMarkTop = top;
    mChannelMarkWidth = width;
    mChannelMarkHeight = height;
}

unsigned char gChannelMarkData[100*100*2] = {0};
void RenderSideWorker::run()
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

    surround_image_t* sideImage = NULL;
    
    
    if (NULL != mCapture)
    {
        //one source from ICapture Module
        sideImage = mCapture->popOneFrame4FocusSource();
        mFocusChannelIndex = mCapture->getFocusChannelIndex();
    }
    else
    {
        //one source from share memory
        if (NULL != mSideSHM)
        {
            unsigned char imageBuf[SHM_SIDE_SIZE] = {};
            if (mSideSHM->readImage(imageBuf, sizeof(imageBuf)) < 0)
            {
                return;
            }
            image_shm_header_t* header = (image_shm_header_t*)imageBuf;
            sideImage = new surround_image_t();
            mFocusChannelIndex = header->channel;
            sideImage->info.width = header->width;
            sideImage->info.height = header->height;
            sideImage->info.pixfmt = header->pixfmt;
            sideImage->info.size = header->size;
            sideImage->timestamp = header->timestamp;
            sideImage->data = imageBuf + sizeof(image_shm_header_t);
        }
    }

    if (NULL == sideImage
        || NULL == sideImage->data)
    {
        return;
    }

#if DEBUG_UPDATE
    clock_t start_draw = clock();
#endif

#if 1
    struct render_surface_t surface;
    surface.srcBuf = (unsigned char*)sideImage->data;
    surface.srcPixfmt = sideImage->info.pixfmt;
    surface.srcWidth = sideImage->info.width;
    surface.srcHeight = sideImage->info.height;
    surface.srcSize = sideImage->info.size;
    surface.dstLeft = mSideImageLeft;
    surface.dstTop = mSideImageTop;
    surface.dstWidth = mSideImageWidth;
    surface.dstHeight = mSideImageHeight;
    drawImage(&surface);
#else

    struct render_surface_t surfaces[2];

    surfaces[0].srcBuf = (unsigned char*)sideImage->data;
    surfaces[0].srcPixfmt = sideImage->info.pixfmt;
    surfaces[0].srcWidth = sideImage->info.width;
    surfaces[0].srcHeight = sideImage->info.height;
    surfaces[0].srcSize = sideImage->info.size;
    surfaces[0].dstLeft = mSideImageLeft;
    surfaces[0].dstTop = mSideImageTop;
    surfaces[0].dstWidth = mSideImageWidth;
    surfaces[0].dstHeight = mSideImageHeight;

    surfaces[1].srcBuf = (unsigned char*)gChannelMarkData;
    surfaces[1].srcPixfmt = V4L2_PIX_FMT_YUYV;
    surfaces[1].srcWidth = mChannelMarkWidth;
    surfaces[1].srcHeight = mChannelMarkHeight;
    surfaces[1].srcSize = surfaces[1].srcWidth*surfaces[1].srcHeight*2;
    surfaces[1].dstLeft = mChannelMarkLeft;
    surfaces[1].dstTop = mChannelMarkTop;
    surfaces[1].dstWidth = mChannelMarkWidth;
    surfaces[1].dstHeight = mChannelMarkHeight;

#if 1
    drawImage(&surfaces[0]);
    drawImage(&surfaces[1]);
#else
    drawMultiImages(surfaces, 2);
#endif

#endif

#if DEBUG_UPDATE
    std::cout << "RenderSideWorker::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << Util::get_system_milliseconds() - sideImage->timestamp
            << ", draw:" << (double)(clock() - start_draw)/CLOCKS_PER_SEC
            << ", channel:" << mFocusChannelIndex
            << " width:"  << sideImage->info.width
            << " height:" << sideImage->info.height
            << " size:" << sideImage->info.size
            << std::endl;
#endif

    delete sideImage;
}

