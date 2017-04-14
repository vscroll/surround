#include "stitchimpl.h"
#include "stitchworker.h"

StitchImpl::StitchImpl(QObject *parent) :
    QObject(parent)
{
    mWorker = new StitchWorker();
}

StitchImpl::~StitchImpl()
{

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
