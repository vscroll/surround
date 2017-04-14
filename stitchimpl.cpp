#include "stitchimpl.h"
#include "stitchworker.h"

StitchImpl::StitchImpl(QObject *parent) :
    QObject(parent)
{

}

StitchImpl::~StitchImpl()
{

}

void StitchImpl::start(ICapture* capture, int fps)
{
    if (NULL == mWorker)
    {
        mWorker = new StitchWorker();
        mWorker->start(capture);

        connect(&mTimer, SIGNAL(timeout()), mWorker, SLOT(onStitch()));
        mTimer.start(1000/fps);

        mWorker->moveToThread(&mThread);
        mThread.start(QThread::TimeCriticalPriority);
    }
}

void StitchImpl::stop()
{
    mTimer.stop();
    mThread.quit();

    if (NULL != mWorker)
    {
        mWorker->stop();
        delete mWorker;
        mWorker = NULL;
    }
}

surround_image_t* StitchImpl::dequeueFullImage()
{
    surround_image_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeueFullImage();
    }
    return image;
}

surround_image_t* StitchImpl::dequeueSmallImage(VIDEO_CHANNEL channel)
{
    surround_image_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeueSmallImage(channel);
    }
    return image;
}
