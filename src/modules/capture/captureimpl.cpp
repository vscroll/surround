#include "captureimpl.h"
#include "captureworkerv4l2.h"

CaptureImpl::CaptureImpl()
{
    mCaptureWorker = new CaptureWorkerV4l2();
}

CaptureImpl::~CaptureImpl()
{
    if (NULL != mCaptureWorker)
    {
        delete mCaptureWorker;
        mCaptureWorker = NULL;
    }
}

void CaptureImpl::setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum)
{
    if (NULL != mCaptureWorker)
    {
        return mCaptureWorker->setCapCapacity(sink, source, channelNum);
    }
}

int CaptureImpl::openDevice(unsigned int channel[], unsigned int channelNum)
{
    if (NULL == mCaptureWorker)
    {
        return -1;
    }

    return mCaptureWorker->openDevice(channel, channelNum);;
}

void CaptureImpl::closeDevice()
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->closeDevice();
    }
}

int CaptureImpl::start(unsigned int fps)
{
    if (NULL == mCaptureWorker)
    {
        return -1;
    }

    return mCaptureWorker->start(1000/fps);
}

void CaptureImpl::stop()
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->stop();
    }
}

int CaptureImpl::getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (NULL == mCaptureWorker)
    {
        return -1;
    }

    return mCaptureWorker->getResolution(channelIndex, width, height);
}

int CaptureImpl::getFPS(unsigned int* fps)
{
    if (NULL == mCaptureWorker)
    {
        return -1;
    }

	return mCaptureWorker->getFPS(fps);
}

surround_images_t* CaptureImpl::popOneFrame()
{
    surround_images_t* pFrame = NULL;
    if (NULL != mCaptureWorker)
    {
        pFrame = mCaptureWorker->popOneFrame();
    }
    return pFrame;
}
