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

int CaptureImpl::setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum)
{
    if (NULL == mCaptureWorker)
    {
        return -1;
    }

    return mCaptureWorker->setCapCapacity(sink, source, channelNum);
}

int CaptureImpl::setFocusSource(unsigned int focusChannelIndex, struct cap_src_t* focusSource)
{
    if (NULL == mCaptureWorker)
    {
        return -1;
    }

    return mCaptureWorker->setFocusSource(focusChannelIndex, focusSource);
}

unsigned int CaptureImpl::getFocusChannelIndex()
{
    if (NULL != mCaptureWorker)
    {
        return mCaptureWorker->getFocusChannelIndex();
    }

    return VIDEO_CHANNEL_FRONT;
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

    return mCaptureWorker->start(fps);
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

int CaptureImpl::getFPS(unsigned int channelIndex, unsigned int* fps)
{
    if (NULL == mCaptureWorker)
    {
        return -1;
    }

	return mCaptureWorker->getFPS(fps);
}

surround_images_t* CaptureImpl::popOneFrame()
{
    surround_images_t* frame = NULL;
    if (NULL != mCaptureWorker)
    {
        frame = mCaptureWorker->popOneFrame();
    }
    return frame;
}

surround_image_t* CaptureImpl::popOneFrame4FocusSource()
{
    surround_image_t* frame = NULL;
    if (NULL != mCaptureWorker)
    {
        frame = mCaptureWorker->popOneFrame4FocusSource();
    }
    return frame;
}

void CaptureImpl::enableCapture()
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->enableCapture();
    }
}
surround_image_t* CaptureImpl::captureOneFrame4FocusSource()
{
    surround_image_t* frame = NULL;
    if (NULL != mCaptureWorker)
    {
        frame = mCaptureWorker->captureOneFrame4FocusSource();
    }
    return frame;
}

surround_image_t* CaptureImpl::popOneFrame(unsigned int channelIndex)
{
    surround_image_t* frame = NULL;
    return frame;
}
