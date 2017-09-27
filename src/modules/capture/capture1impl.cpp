#include "capture1impl.h"
#include "capture1workerv4l2.h"
#include <stdio.h>

Capture1Impl::Capture1Impl(unsigned int channelNum)
{
    mChannelNum = channelNum;
    mFocusChannelIndex = VIDEO_CHANNEL_FRONT;

    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        mCaptureWorker[i] = new Capture1WorkerV4l2();
    }
}

Capture1Impl::~Capture1Impl()
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            delete mCaptureWorker[i];
            mCaptureWorker[i] = NULL;
        }
    }
}

int Capture1Impl::setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum)
{
    int ret = -1;
    unsigned int num = mChannelNum <= channelNum ? mChannelNum:channelNum;
    for (unsigned int i = 0; i < num; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            ret = mCaptureWorker[i]->setCapCapacity(&sink[i], &source[i]);
            if (ret  < 0)
            {
                break;
            }
        }
    }

    return ret;
}

int Capture1Impl::setFocusSource(unsigned int focusChannelIndex, struct cap_src_t* focusSource)
{
    if (focusChannelIndex >= mChannelNum)
    {
        return -1;
    }
    
    struct cap_src_t oldFocusSource;
    if (mFocusChannelIndex != focusChannelIndex)
    {
        //clear
        if (NULL != mCaptureWorker[mFocusChannelIndex])
        {
            mCaptureWorker[mFocusChannelIndex]->getFocusSource(&oldFocusSource);
            mCaptureWorker[mFocusChannelIndex]->clearFocusSource();
        }  
    }

    mFocusChannelIndex = focusChannelIndex;
    if (NULL != mCaptureWorker[mFocusChannelIndex])
    {
        if (NULL == focusSource)
        {
            return mCaptureWorker[mFocusChannelIndex]->setFocusSource(&oldFocusSource);
        }
        else
        {
            return mCaptureWorker[mFocusChannelIndex]->setFocusSource(focusSource);
        }
    }

    return -1;
}

unsigned int Capture1Impl::getFocusChannelIndex()
{
    return mFocusChannelIndex;
}

int Capture1Impl::openDevice(unsigned int channel[], unsigned int channelNum)
{
    mChannelNum = channelNum < VIDEO_CHANNEL_SIZE ? channelNum : VIDEO_CHANNEL_SIZE;

    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            mCaptureWorker[i]->openDevice(channel[i]);
        }
    }

    return 0;
}

void Capture1Impl::closeDevice()
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            mCaptureWorker[i]->closeDevice();
        }
    }
}

int Capture1Impl::start(unsigned int fps)
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            mCaptureWorker[i]->start(fps);
        }
    }

    return 0;
}

void Capture1Impl::stop()
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            mCaptureWorker[i]->stop();
        }
    }
}

int Capture1Impl::getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (channelIndex >= mChannelNum)
    {
        return -1;
    }

    if (NULL == mCaptureWorker[channelIndex])
    {
        return -1;
    }

    return mCaptureWorker[channelIndex]->getResolution(width, height);
}

int Capture1Impl::getFPS(unsigned int channelIndex, unsigned int* fps)
{
    if (channelIndex >= mChannelNum)
    {
        return -1;
    }

    if (NULL == mCaptureWorker[channelIndex])
    {
        return -1;
    }

	return mCaptureWorker[channelIndex]->getFPS(fps);
}

surround_images_t* Capture1Impl::popOneFrame()
{
    surround_images_t* pFrame = new surround_images_t();
    pFrame->timestamp = 0;
    bool isFull = true;
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        surround_image_t* tmp = mCaptureWorker[i]->popOneFrame();
        if (NULL != tmp)
        {
            pFrame->frame[i].data = tmp->data;
            pFrame->frame[i].pAddr = tmp->pAddr;
            pFrame->frame[i].info.width = tmp->info.width;
            pFrame->frame[i].info.height = tmp->info.height;
            pFrame->frame[i].info.pixfmt = tmp->info.pixfmt;
            pFrame->frame[i].info.size = tmp->info.size;

            // get the earliest one
            if (pFrame->timestamp == 0
                || pFrame->timestamp > tmp->timestamp)
            {
                pFrame->timestamp = tmp->timestamp;
            }
            delete tmp;
        }
        else
        {
            isFull = false;
            break;
        }
    }

    if (!isFull)
    {
        delete pFrame;
        pFrame = NULL;
    }

    return pFrame;
}

surround_image_t* Capture1Impl::popOneFrame4FocusSource()
{
    surround_image_t* frame = NULL;
    if (NULL != mCaptureWorker[mFocusChannelIndex])
    {
        frame = mCaptureWorker[mFocusChannelIndex]->popOneFrame4FocusSource();
    }
    return frame;
}

void Capture1Impl::enableCapture()
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            mCaptureWorker[i]->enableCapture();
        }
    }
}
surround_image_t* Capture1Impl::captureOneFrame4FocusSource()
{
    surround_image_t* frame = NULL;
    if (NULL != mCaptureWorker[mFocusChannelIndex])
    {
        frame = mCaptureWorker[mFocusChannelIndex]->captureOneFrame4FocusSource();
    }
    return frame;
}

surround_image_t* Capture1Impl::popOneFrame(unsigned int channelIndex)
{
    if (channelIndex >= mChannelNum)
    {
        return NULL;
    }

    if (NULL == mCaptureWorker[channelIndex])
    {
        return NULL;
    }

    surround_image_t* frame = NULL;
    if (NULL != mCaptureWorker[channelIndex])
    {
        frame = mCaptureWorker[channelIndex]->popOneFrame();
    }
    return frame;
}
