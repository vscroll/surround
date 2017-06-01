#include "capture1workerbase.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

Capture1WorkerBase::Capture1WorkerBase()
{
	memset(&mSink, 0, sizeof(mSink));
	memset(&mSource, 0, sizeof(mSource));

    mVideoFd = -1;
    mVideoChannel = 0;

    pthread_mutex_init(&mMutexQueue, NULL);

    //focus source
    memset(&mFocusSource, 0, sizeof(mFocusSource));
    pthread_mutex_init(&mMutexFocusSourceQueue, NULL);

    //capture soure
    mEnableCapture = false;
    memset(&mCaptureFrame4FocusSource, 0, sizeof(mCaptureFrame4FocusSource)); 
 
    mLastCallTime = 0;

    mRealFPS = 0;
    mStartStatTime = 0;
    mStatDuration = 0;
    mRealFrameCount = 0;
}

Capture1WorkerBase::~Capture1WorkerBase()
{

}

void Capture1WorkerBase::setCapCapacity(struct cap_sink_t* sink, struct cap_src_t* source)
{
    memcpy(&mSink, sink, sizeof(mSink));
    memcpy(&mSource, source, sizeof(mSource));
}

void Capture1WorkerBase::setFocusSource(struct cap_src_t* focusSource)
{
    memcpy(&mFocusSource, focusSource, sizeof(mFocusSource));
}

void Capture1WorkerBase::clearFocusSource()
{
    memset(&mFocusSource, 0, sizeof(mFocusSource));

    clearFocusSourceQueue();
}

void Capture1WorkerBase::clearFocusSourceQueue()
{
    pthread_mutex_lock(&mMutexFocusSourceQueue);
    int size = mFocusSourceQueue.size();
    for (int i = 0; i < size; ++i)
    {
        struct surround_image_t* surroundImage = mFocusSourceQueue.front();
        mFocusSourceQueue.pop();
        if (NULL != surroundImage)
        {
            delete surroundImage;
        }
    }
    pthread_mutex_unlock(&mMutexFocusSourceQueue); 
}

int Capture1WorkerBase::getResolution(unsigned int* width, unsigned int* height)
{
    *width = mSource.width;
    *height = mSource.height;

    return 0;
}

int Capture1WorkerBase::getFPS(unsigned int* fps)
{
    if (mRealFPS > 0)
    {
        *fps = mRealFPS;
	    return 0;
    }

    return -1;
}

unsigned int Capture1WorkerBase::getFrameCount()
{
    pthread_mutex_lock(&mMutexQueue);
    unsigned int size = mSurroundImageQueue.size();
    pthread_mutex_unlock(&mMutexQueue);
    return size;
}

surround_image_t* Capture1WorkerBase::popOneFrame()
{

    struct surround_image_t* surroundImage = NULL;
    pthread_mutex_lock(&mMutexQueue);
    if (mSurroundImageQueue.size() > 0)
    {
        surroundImage = mSurroundImageQueue.front();
        mSurroundImageQueue.pop();
    }
    pthread_mutex_unlock(&mMutexQueue);

    return surroundImage;
}

surround_image_t* Capture1WorkerBase::popOneFrame4FocusSource()
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

void Capture1WorkerBase::enableCapture()
{
    mEnableCapture = true;
}

surround_image_t* Capture1WorkerBase::captureOneFrame4FocusSource()
{
    surround_image_t* frame = NULL;
    if (mEnableCapture)
    {
        frame = &mCaptureFrame4FocusSource;
    }
    return frame;
}
