#include "rendersideworker.h"
#include <string.h>
#include <iostream>

RenderSideWorker::RenderSideWorker()
{
    mCapture = NULL;
}

RenderSideWorker::~RenderSideWorker()
{

}

void RenderSideWorker::init(ICapture* capture)
{
    mCapture = capture;
}

void RenderSideWorker::run()
{
    if (NULL == mCapture)
    {
        return;
    }
}
