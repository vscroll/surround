#include "renderimpl.h"
#include "renderpano2dworker.h"
#include "IStitch.h"

RenderImpl::RenderImpl()
{

}

RenderImpl::~RenderImpl()
{

}

int RenderImpl::start(IStitch *stitch,
                unsigned int fps,
		unsigned int pano2DLeft,
		unsigned int pano2DTop,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight)
{
    int ret = -1;
    if (NULL == mWorker)
    {
        mWorker = new RenderPano2DWorker();
        ret = mWorker->init(stitch,	
		pano2DLeft,
		pano2DTop,
		pano2DWidth,
		pano2DHeight);
        if (ret < 0)
        {
	    return ret;
        }

	ret = mWorker->start(1000/fps);
    }

    return ret;
}

void RenderImpl::stop()
{
    if (NULL != mWorker)
    {
        mWorker->stop();
        mWorker->uninit();
        delete mWorker;
        mWorker = NULL;
    }
}
