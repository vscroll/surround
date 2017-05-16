#include "renderimpl.h"
#include "renderpanoworker.h"
#include "rendersideworker.h"

RenderImpl::RenderImpl()
{
    mSideWorker = new RenderSideWorker();
    mPanoWorker = new RenderPanoWorker();
}

RenderImpl::~RenderImpl()
{

}

int RenderImpl::init(
        ICapture* capture,
        IPanoImage* panoImage,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight,
		unsigned int panoLeft,
		unsigned int panoTop,
		unsigned int panoWidth,
		unsigned int panoHeight)
{
    
    if (NULL == mSideWorker
        || NULL == mPanoWorker)
    {
        return -1;
    }

    mSideWorker->init(capture);
    if (mSideWorker->openDevice(sideLeft, sideTop, sideWidth, sideHeight) < 0)
    {
        mSideWorker->closeDevice();
        return -1;
    }


    mPanoWorker->init(panoImage);
    if (mPanoWorker->openDevice(panoLeft, panoTop, panoWidth, panoHeight) < 0)
    {
        mSideWorker->closeDevice();
        mPanoWorker->closeDevice();
        return -1;
    }

    return 0;
}

int RenderImpl::start(unsigned int fps)
{
    if (NULL == mSideWorker
        || NULL == mPanoWorker)
    {
        return -1;
    }

    mSideWorker->start(1000/fps);
    mPanoWorker->start(1000/fps);

    return 0;
}

void RenderImpl::stop()
{
    if (NULL != mSideWorker)
    {
        mSideWorker->stop();
        mSideWorker->closeDevice();
    }

    if (NULL != mPanoWorker)
    {
        mPanoWorker->stop();
        mPanoWorker->closeDevice();
    }
}
