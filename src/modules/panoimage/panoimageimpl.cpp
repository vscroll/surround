#include "panoimageimpl.h"
#include "stitchworker.h"

PanoImageImpl::PanoImageImpl()
{
    mWorker = new StitchWorker();
}

PanoImageImpl::~PanoImageImpl()
{
    if (NULL != mWorker)
    {
        delete mWorker;
        mWorker = NULL;
    }
}

int PanoImageImpl::init(
        ICapture* capture,
        unsigned int inWidth,
		unsigned int inHeight,
		unsigned int inPixfmt,        
		unsigned int panoWidth,
		unsigned int panoHeight,
		unsigned int panoPixfmt,
		char* algoCfgFilePath,
		bool enableOpenCL)
{
    if (NULL == mWorker)
    {
        return -1;
    }

    return mWorker->init(
                capture,
                inWidth,
                inHeight,
                inPixfmt,
                panoWidth,
                panoHeight,
                panoPixfmt,
		        algoCfgFilePath,
		        enableOpenCL);
}

int PanoImageImpl::start(unsigned int fps)
{
    if (NULL == mWorker)
    {
        return -1;
    }

    return mWorker->start(1000/fps);
}

void PanoImageImpl::stop()
{
    if (NULL == mWorker)
    {
        return;
    }

    mWorker->stop();
}

void PanoImageImpl::queueImages(surround_images_t* surroundImages)
{
    if (NULL != mWorker)
    {
        mWorker->queueImages(surroundImages);
    }
}

surround_image_t* PanoImageImpl::dequeuePanoImage()
{
    surround_image_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeuePanoImage();
    }
    return image;
}
