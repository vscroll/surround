#include "stitchimpl.h"
#include "stitchworker.h"

StitchImpl::StitchImpl(QObject *parent) :
    QObject(parent)
{
    mWorker = new StitchWorker();
}

void StitchImpl::start(ICapture* capture)
{
    if (NULL != mWorker)
    {
        mWorker->start(capture);
    }
}

void StitchImpl::stop()
{
    if (NULL != mWorker)
    {
        mWorker->stop();
    }
}

surround_image1_t* StitchImpl::dequeueFullImage()
{
    surround_image1_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeueFullImage();
    }
    return image;
}

surround_image1_t* StitchImpl::dequeueSmallImage(VIDEO_CHANNEL channel)
{
    surround_image1_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeueSmallImage(channel);
    }
    return image;
}
