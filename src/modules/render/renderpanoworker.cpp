#include "renderpanoworker.h"
#include <string.h>
#include <iostream>

RenderPanoWorker::RenderPanoWorker()
{
    mPanoImage = NULL;
}

RenderPanoWorker::~RenderPanoWorker()
{

}

void RenderPanoWorker::init(IPanoImage* panoImage)
{
    mPanoImage = panoImage;
}

void RenderPanoWorker::run()
{
    if (NULL == mPanoImage)
    {
        return;
    }
}
