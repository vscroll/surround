#include "renderimpl.h"
#include "renderpanoworker.h"
#include "rendersideworker.h"
#include "rendermarkworker.h"

RenderImpl::RenderImpl()
{
    mSideWorker = new RenderSideWorker();
    mMarkWorker = new RenderMarkWorker();
    mPanoWorker = new RenderPanoWorker();
}

RenderImpl::~RenderImpl()
{
}

void RenderImpl::setCaptureModule(ICapture* capture)
{
    if (NULL != mSideWorker)
    {
        mSideWorker->setCaptureModule(capture);
    }
}

void RenderImpl::setSideImageCrop(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height)
{
    if (NULL != mSideWorker)
    {
        mSideWorker->setSideImageCrop(left, top, width, height);
    }
}

void RenderImpl::setSideImageRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height)
{
    if (NULL != mSideWorker)
    {
        mSideWorker->setSideImageRect(left, top, width, height);
    }
}

void RenderImpl::setMarkRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height)
{
    if (NULL != mMarkWorker)
    {
        mMarkWorker->setMarkRect(left, top, width, height);
    }
}

void RenderImpl::setPanoImageModule(IPanoImage* panoImage)
{
    if (NULL != mPanoWorker)
    {
        mPanoWorker->setPanoImageModule(panoImage);
    }
}

void RenderImpl::setPanoImageRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height)
{
    if (NULL != mPanoWorker)
    {
        mPanoWorker->setPanoImageRect(left, top, width, height);
    }
}

int RenderImpl::startRenderSide(unsigned int fps)
{
    if (NULL == mSideWorker)
    {
        return -1;
    }

    unsigned int sideLeft;
    unsigned int sideTop;
    unsigned int sideWidth;
    unsigned int sideHeight;
    mSideWorker->getSideImageRect(&sideLeft, &sideTop, &sideWidth, &sideHeight);
    if (mSideWorker->openDevice(sideLeft, sideTop, sideWidth, sideHeight) < 0)
    {
        mSideWorker->closeDevice();
    }
    mSideWorker->start(fps);

    return 0;
}

int RenderImpl::startRenderMark(unsigned int fps)
{
    if (NULL == mMarkWorker)
    {
        return -1;
    }

    unsigned int markLeft;
    unsigned int markTop;
    unsigned int markWidth;
    unsigned int markHeight;
    mMarkWorker->getMarkRect(&markLeft, &markTop, &markWidth, &markHeight);
    if (mMarkWorker->openDevice(markLeft, markTop, markWidth, markHeight) < 0)
    {
        mMarkWorker->closeDevice();
    }
    mMarkWorker->start(fps);

    return 0;
}

int RenderImpl::startRenderPano(unsigned int fps)
{
    if (NULL == mPanoWorker)
    {
        return -1;
    }

    unsigned int panoLeft;
    unsigned int panoTop;
    unsigned int panoWidth;
    unsigned int panoHeight;
    mPanoWorker->getPanoImageRect(&panoLeft, &panoTop, &panoWidth, &panoHeight);
    if (mPanoWorker->openDevice(panoLeft, panoTop, panoWidth, panoHeight) < 0)
    {
        mPanoWorker->closeDevice();
    }
    mPanoWorker->start(fps);

    return 0;
}

int RenderImpl::start(unsigned int fps)
{
    startRenderSide(fps);
    startRenderMark(fps);
    startRenderPano(fps);

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
