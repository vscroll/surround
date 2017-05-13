#include "panoimpl.h"
#include "stitchworker.h"

PanoImpl::PanoImpl()
{
    mWorker = new StitchWorker();
}

PanoImpl::~PanoImpl()
{
    if (NULL != mWorker)
    {
        delete mWorker;
        mWorker = NULL;
    }
}

int PanoImpl::init(unsigned int inWidth,
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

    return mWorker->init(inWidth,
                inHeight,
                inPixfmt,
                panoWidth,
                panoHeight,
                panoPixfmt,
		        algoCfgFilePath,
		        enableOpenCL);
}

int PanoImpl::start(unsigned int fps)
{
    if (NULL == mWorker)
    {
        return -1;
    }

    return mWorker->start(1000/fps);
}

void PanoImpl::stop()
{
    if (NULL == mWorker)
    {
        return;
    }

    mWorker->stop();
}

void PanoImpl::queueImages(surround_images_t* surroundImages)
{
    if (NULL != mWorker)
    {
        mWorker->queueImages(surroundImages);
    }
}

surround_image_t* PanoImpl::dequeuePanoImage()
{
    surround_image_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeuePanoImage();
    }
    return image;
}
