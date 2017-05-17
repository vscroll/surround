#include "rendersideworker.h"
#include <string.h>
#include <iostream>
#include "common.h"
#include "ICapture.h"
#include "util.h"

RenderSideWorker::RenderSideWorker()
{
    mCapture = NULL;
}

RenderSideWorker::~RenderSideWorker()
{
    mLastCallTime = 0;
}

void RenderSideWorker::init(ICapture* capture)
{
    mCapture = capture;
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

    if (NULL == mCapture)
    {
        return;
    }

    surround_image_t* sideImage = mCapture->popOneFrame4FocusSource();
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

