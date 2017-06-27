#include "captureworkerbase.h"
#include <string.h>
#include <stdio.h>
#include <iostream>

CaptureWorkerBase::CaptureWorkerBase()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mVideoFd[i] = -1;

	    memset(&mSink[i], 0, sizeof(mSink[i]));
	    memset(&mSource[i], 0, sizeof(mSource[i]));

        mChannel[i] = 0;
    }
    
    mVideoChannelNum = 0;

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        pthread_mutex_init(&mMutexQueue[i], NULL);
    }

    //focus source
    mFocusChannelIndex = VIDEO_CHANNEL_FRONT;
    memset(&mFocusSource, 0, sizeof(mFocusSource));
    pthread_mutex_init(&mMutexFocusSourceQueue, NULL);

    mEnableCapture = false;
    memset(&mCaptureFrame4FocusSource, 0, sizeof(mCaptureFrame4FocusSource));  

    mLastCallTime = 0;

    mRealFPS = 0;
    mStartStatTime = 0;
    mStatDuration = 0;
    mRealFrameCount = 0;
}

CaptureWorkerBase::~CaptureWorkerBase()
{
}


int CaptureWorkerBase::setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum)
{
#if 0
    for (unsigned int i = 0; i < channelNum; ++i)
    {
        if (sink[i].pixfmt != source[i].pixfmt)
        {
            std::cout << "CaptureWorkerBase::setCapCapacity:"
                    << " not support convert sink to source"
					<< ", sink width:" << sink[i].width
					<< ", sink height:" << sink[i].height
					<< ", source width:" << source[i].width
					<< ", source height:" << source[i].height
                    << std::endl;
            return -1;
        }
    }
#endif

    mVideoChannelNum = channelNum <= VIDEO_CHANNEL_SIZE ? channelNum: VIDEO_CHANNEL_SIZE;
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
	    memcpy(&mSink[i], &sink[i], sizeof(mSink[i]));
	    memcpy(&mSource[i], &source[i], sizeof(mSource[i]));
    }

    return 0;
}

int CaptureWorkerBase::setFocusSource(unsigned int focusChannelIndex, struct cap_src_t* focusSource)
{
    if (focusChannelIndex >= VIDEO_CHANNEL_SIZE)
    {
        return -1;
    }

    if (isNeedConvert(&mSink[focusChannelIndex], focusSource))
    {
        std::cout << "CaptureWorkerBase::setFocusSource:"
                << " not support convert sink to source"
				<< ", sink width:" << mSink[focusChannelIndex].width
				<< ", sink height:" << mSink[focusChannelIndex].height
				<< ", source width:" << focusSource->width
				<< ", source height:" << focusSource->height
                << std::endl;
        return -1;
    }

    mFocusChannelIndex = focusChannelIndex;
    memcpy(&mFocusSource, focusSource, sizeof(mFocusSource));

    return 0;
}

unsigned int CaptureWorkerBase::getFocusChannelIndex()
{
    return mFocusChannelIndex;
}

int CaptureWorkerBase::getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (channelIndex >= VIDEO_CHANNEL_SIZE)
    {
        return -1;
    }

    *width = mSource[channelIndex].width;
    *height = mSource[channelIndex].height;

    return 0;
}

int CaptureWorkerBase::getFPS(unsigned int* fps)
{
#if 0
    unsigned int interval = getInterval();
    if (interval > 0)
    {
        *fps = 1000/interval;
	return 0;
    }
#else
    if (mRealFPS > 0)
    {
        *fps = mRealFPS;
	    return 0;
    }

#endif
    return -1;
}

surround_images_t* CaptureWorkerBase::popOneFrame()
{
    struct surround_image_t* image[VIDEO_CHANNEL_SIZE] = {NULL};
    bool isFull = true;
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        pthread_mutex_lock(&mMutexQueue[i]);
        if (mSurroundImageQueue[i].size() > 0)
        {
            image[i] = mSurroundImageQueue[i].front();
            mSurroundImageQueue[i].pop();
        }
        else
        {
            isFull = false;
        }
        pthread_mutex_unlock(&mMutexQueue[i]);
    }

    if (!isFull)
    {
        for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            if (NULL != image[i])
            {
                delete image[i];
                image[i] = NULL;
            }
        }
        return NULL;
    }

    struct surround_images_t* surroundImage = new surround_images_t();
    surroundImage->timestamp = 0;
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        surroundImage->frame[i].timestamp = image[i]->timestamp;
        surroundImage->frame[i].info.width = image[i]->info.width;
        surroundImage->frame[i].info.height = image[i]->info.height;
        surroundImage->frame[i].info.size = image[i]->info.size;
        surroundImage->frame[i].info.pixfmt = image[i]->info.pixfmt;
        surroundImage->frame[i].data = image[i]->data;
        if (surroundImage->timestamp > image[i]->timestamp)
        {
            surroundImage->timestamp = image[i]->timestamp;
        }
    }    

    return surroundImage;
}

surround_image_t* CaptureWorkerBase::popOneFrame4FocusSource()
{
    struct surround_image_t* surroundImage = NULL;
    pthread_mutex_lock(&mMutexFocusSourceQueue);
    if (mFocusSourceQueue.size() > 0)
    {
        surroundImage = mFocusSourceQueue.front();
        mFocusSourceQueue.pop();
    }
    pthread_mutex_unlock(&mMutexFocusSourceQueue);

    return surroundImage;
}

void CaptureWorkerBase::enableCapture()
{
    mEnableCapture = true;
}

surround_image_t* CaptureWorkerBase::captureOneFrame4FocusSource()
{
    surround_image_t* frame = NULL;
    if (mEnableCapture)
    {
        frame = &mCaptureFrame4FocusSource;
    }
    return frame;
}

surround_image_t* CaptureWorkerBase::popOneFrame(unsigned int channelIndex)
{
    surround_image_t* frame = NULL;
    if (channelIndex < VIDEO_CHANNEL_SIZE)
    {
        pthread_mutex_lock(&mMutexQueue[channelIndex]);
        if (mSurroundImageQueue[channelIndex].size() > 0)
        {
            frame = mSurroundImageQueue[channelIndex].front();
            mSurroundImageQueue[channelIndex].pop();
        }
        pthread_mutex_unlock(&mMutexQueue[channelIndex]);        
    }
    return frame;
}

void CaptureWorkerBase::clearOverstock()
{
    pthread_mutex_lock(&mMutexFocusSourceQueue);
    int focus_size = mFocusSourceQueue.size();
    if (focus_size > OVERSTOCK_SIZE)
    {
        struct surround_image_t* surroundImage = mFocusSourceQueue.front();
        mFocusSourceQueue.pop();
        if (NULL != surroundImage)
        {
            delete surroundImage;
            surroundImage = NULL;
        }
    }
    pthread_mutex_unlock(&mMutexFocusSourceQueue); 

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        pthread_mutex_lock(&mMutexQueue[i]);
        int size = mSurroundImageQueue[i].size();
        if (size > OVERSTOCK_SIZE)
        {
            for (int j = 0; j < size; ++j)
            {
                struct surround_image_t* surroundImage = mSurroundImageQueue[i].front();
                mSurroundImageQueue[i].pop();
                if (NULL != surroundImage)
                {
                    delete surroundImage;
                    surroundImage = NULL;
                }
            }
        }
        pthread_mutex_unlock(&mMutexQueue[i]);
    }
}

bool CaptureWorkerBase::isNeedConvert(struct cap_sink_t* sink, struct cap_src_t* source)
{
    return (sink->pixfmt != source->pixfmt
                    || sink->width != source->width
                    || sink->height != source->height);
}
