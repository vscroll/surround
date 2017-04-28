#include "stitchimpl.h"
#include "stitchworker.h"

StitchImpl::StitchImpl()
{
    mWorker = NULL;
}

StitchImpl::~StitchImpl()
{

}

void StitchImpl::start(ICapture *capture,
		unsigned int fps,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		char* configFilePath,
		bool enableOpenCL)
{
    if (NULL == mWorker)
    {
        mWorker = new StitchWorker();
        mWorker->init(capture,		
		pano2DWidth,
		pano2DHeight,
		configFilePath,
		enableOpenCL);

	mWorker->start(1000/fps);
    }
}

void StitchImpl::stop()
{
    if (NULL != mWorker)
    {
        mWorker->stop();
        delete mWorker;
        mWorker = NULL;
    }
}

surround_image_t* StitchImpl::dequeuePano2DImage()
{
    surround_image_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeuePano2DImage();
    }
    return image;
}

surround_image_t* StitchImpl::dequeueSideImage(unsigned int channelIndex)
{
    surround_image_t* image = NULL;
    if (NULL != mWorker)
    {
        image = mWorker->dequeueSideImage(channelIndex);
    }
    return image;
}
