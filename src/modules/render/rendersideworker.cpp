#include "rendersideworker.h"
#include <string.h>
#include <iostream>
#include "common.h"
#include "ICapture.h"
#include "util.h"
#include "imageshm.h"

RenderSideWorker::RenderSideWorker()
{
    mCapture = NULL;
    mSideSHM = NULL;
}

RenderSideWorker::~RenderSideWorker()
{
    mLastCallTime = 0;
}

void RenderSideWorker::init(ICapture* capture)
{
    mCapture = capture;
    if (NULL == capture)
    {
        mSideSHM = new ImageSHM();
        mSideSHM->create((key_t)SHM_SIDE_ID, SHM_SIDE_SIZE);
    }
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

    surround_image_t* sideImage = NULL;
    
    
    if (NULL != mCapture)
    {
        //one source from ICapture Module
        sideImage = mCapture->popOneFrame4FocusSource();
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

    if (NULL == sideImage->data)
    {
        return;
    }

#if DEBUG_UPDATE
    clock_t start_draw = clock();
#endif

    drawImage((unsigned char*)sideImage->data,
            sideImage->info.pixfmt,
            sideImage->info.width,
            sideImage->info.height,
            sideImage->info.size);

#if DEBUG_UPDATE
    std::cout << "RenderSideWorker::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << Util::get_system_milliseconds() - sideImage->timestamp
            << ", draw:" << (double)(clock() - start_draw)/CLOCKS_PER_SEC
            << " width:"  << sideImage->info.width
            << " height:" << sideImage->info.height
            << " size:" << sideImage->info.size
            << std::endl;
#endif

    delete sideImage;
}

