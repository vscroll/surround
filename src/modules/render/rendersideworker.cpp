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
    mImageSHM = NULL;

    mSideImageCropLeft = 0;
    mSideImageCropTop = 0;
    mSideImageCropWidth = 0;
    mSideImageCropHeight = 0;

    mSideImageLeft = 0;
    mSideImageTop = 0;
    mSideImageWidth = 0;
    mSideImageHeight = 0;

    mFocusChannelIndex = VIDEO_CHANNEL_FRONT;

    mLastCallTime = 0;
}

RenderSideWorker::~RenderSideWorker()
{
    if (NULL != mImageSHM)
    {
        mImageSHM->destroy();
        delete mImageSHM;
        mImageSHM = NULL;
    }
}

void RenderSideWorker::setCaptureModule(ICapture* capture)
{
    mCapture = capture;
    if (NULL == capture)
    {
        mImageSHM = new ImageSHM();
        mImageSHM->create((key_t)SHM_FOCUS_SOURCE_ID, SHM_FOCUS_SOURCE_SIZE);
    }
}

void RenderSideWorker::setSideImageCrop(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height)
{
    mSideImageCropLeft = left;
    mSideImageCropTop = top;
    mSideImageCropWidth = width;
    mSideImageCropHeight = height;
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

    unsigned char imageBuf[SHM_FOCUS_SOURCE_SIZE] = {};
    surround_image_t* sideImage = NULL; 
    
    if (NULL != mCapture)
    {
        //one source may be come from ICapture Module
        sideImage = mCapture->popOneFrame4FocusSource();
        mFocusChannelIndex = mCapture->getFocusChannelIndex();
    }
    else
    {
        //one source may be come from share memory
        if (NULL != mImageSHM)
        {
            if (mImageSHM->readSource(imageBuf, sizeof(imageBuf)) < 0)
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

    if (NULL == sideImage)
    {
        return;
    }

    if (V4L2_PIX_FMT_YUYV != sideImage->info.pixfmt
        && V4L2_PIX_FMT_UYVY != sideImage->info.pixfmt)
    {
        delete sideImage;
        sideImage = NULL;
        return;
    }

#if DEBUG_UPDATE
    clock_t start_draw = clock();
#endif

    struct render_surface_t surface;
    surface.srcBuf = (unsigned char*)sideImage->data;
    surface.srcPixfmt = sideImage->info.pixfmt;
    if (0 == mSideImageCropWidth
        || 0 == mSideImageCropHeight
        || (mSideImageCropLeft + mSideImageCropWidth) >= sideImage->info.width
        || (mSideImageCropTop + mSideImageCropHeight) >= sideImage->info.height)
    {
        surface.srcLeft = 0;
        surface.srcTop = 0;
        surface.srcWidth = sideImage->info.width;
        surface.srcHeight = sideImage->info.height;
    }
    else
    {
        surface.srcLeft = mSideImageCropLeft;
        surface.srcTop = mSideImageCropTop;
        surface.srcWidth = mSideImageCropWidth;
        surface.srcHeight = mSideImageCropHeight;
    }

	surface.srcStride = sideImage->info.width;
    surface.srcSize = sideImage->info.size;
    surface.dstLeft = mSideImageLeft;
    surface.dstTop = mSideImageTop;
    surface.dstWidth = mSideImageWidth;
    surface.dstHeight = mSideImageHeight;
    drawImage(&surface);

    statFPS();

#if DEBUG_UPDATE
    std::cout << "RenderSideWorker::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)(Util::get_system_milliseconds() - sideImage->timestamp)/1000
            << ", draw:" << (double)(clock() - start_draw)/CLOCKS_PER_SEC
            << ", channel:" << mFocusChannelIndex
            << " width:"  << sideImage->info.width
            << " height:" << sideImage->info.height
            << " fps:" << mRealFPS
            << std::endl;
#endif

    delete sideImage;
}

