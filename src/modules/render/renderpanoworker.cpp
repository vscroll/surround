#include "renderpanoworker.h"
#include <string.h>
#include <iostream>
#include "common.h"
#include "imageshm.h"
#include "util.h"

RenderPanoWorker::RenderPanoWorker()
{
    mPanoImage = NULL;
}

RenderPanoWorker::~RenderPanoWorker()
{
}

void RenderPanoWorker::setPanoImageModule(IPanoImage* panoImage)
{
    mPanoImage = panoImage;
    if (NULL == panoImage)
    {
        mPanoSHM = new ImageSHM();
        mPanoSHM->create((key_t)SHM_PANO_SOURCE_ID, SHM_PANO_SOURCE_SIZE);
    }
}

void RenderPanoWorker::setPanoImageRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height)
{
    mPanoImageLeft = left;
    mPanoImageTop = top;
    mPanoImageWidth = width;
    mPanoImageHeight = height;
}

void RenderPanoWorker::getPanoImageRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height)
{
    *left = mPanoImageLeft;
    *top = mPanoImageTop;
    *width = mPanoImageWidth;
    *height = mPanoImageHeight;
}

void RenderPanoWorker::run()
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

    surround_image_t* panoImage = NULL;
    if (NULL != mPanoImage)
    {
    }
    else
    {
        //one source from share memory
        if (NULL != mPanoSHM)
        {
#if 0
            unsigned char imageBuf[SHM_PANO_SIZE] = {};
            if (mPanoSHM->readSource(imageBuf, sizeof(imageBuf)) < 0)
            {
                return;
            }
            image_shm_header_t* header = (image_shm_header_t*)imageBuf;
            panoImage = new surround_image_t();
            panoImage->info.width = header->width;
            panoImage->info.height = header->height;
            panoImage->info.pixfmt = header->pixfmt;
            panoImage->info.size = header->size;
            panoImage->timestamp = header->timestamp;
            panoImage->data = imageBuf + sizeof(image_shm_header_t);
#endif
        }
    }

    if (NULL == panoImage
        || NULL == panoImage->data)
    {
        return;
    }

#if DEBUG_UPDATE
    clock_t start_draw = clock();
#endif

    struct render_surface_t surface;
    surface.srcBuf = (unsigned char*)panoImage->data;
    surface.srcPixfmt = panoImage->info.pixfmt;
    surface.srcWidth = panoImage->info.width;
    surface.srcHeight = panoImage->info.height;
    surface.dstLeft = mPanoImageLeft;
    surface.dstTop = mPanoImageTop;
    surface.dstWidth = mPanoImageWidth;
    surface.dstHeight = mPanoImageHeight;
    drawImage(&surface);

#if DEBUG_UPDATE
    std::cout << "RenderPanoWorker::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << Util::get_system_milliseconds() - panoImage->timestamp
            << ", draw:" << (double)(clock() - start_draw)/CLOCKS_PER_SEC
            << " width:"  << panoImage->info.width
            << " height:" << panoImage->info.height
            << " size:" << panoImage->info.size
            << std::endl;
#endif

    delete panoImage;
}
