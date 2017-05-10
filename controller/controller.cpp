#include "controller.h"
#include "ICapture.h"
#include "IStitch.h"
#include "stitchimpl.h"
#include "capture1impl.h"
#include "capture4impl.h"
#include "renderimpl.h"

#define G2D_RENDER 0

Controller::Controller()
{
    mCapture = NULL;
    mStitch = NULL;
    mRender = NULL;
}

Controller::~Controller()
{

}

void Controller::init(unsigned int channel[], struct cap_info_t capInfo[], unsigned int channelNum)
{
    if (NULL == mCapture)
    {
#if CAPTURE_4_CHANNEL_ONCE
        mCapture = new Capture4Impl();
#else
        mCapture = new Capture1Impl();
#endif
        mCapture->openDevice(channel, capInfo, channelNum);
    }

    if (NULL == mStitch)
    {
        mStitch = new StitchImpl();
    }

#if G2D_RENDER
    if (NULL == mRender)
    {
        mRender = new RenderImpl();
    }
#endif
}

void Controller::uninit()
{
    if (NULL != mCapture)
    {
         delete mCapture;
         mCapture = NULL;
    }

    if (NULL != mStitch)
    {
        delete mStitch;
        mStitch = NULL;
    }
#if G2D_RENDER
    if (NULL != mRender)
    {
        delete mRender;
        mRender = NULL;
    }
#endif
}

void Controller::start(unsigned int fps,
		unsigned int pano2DLeft,
		unsigned int pano2DTop,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight,
		char* configFilePath,
		bool enableOpenCL)
{

#if G2D_RENDER
   if (NULL != mRender)
   {
       mRender->start(mStitch,
			fps,
			pano2DLeft,
			pano2DTop,
			pano2DWidth,
			pano2DHeight,
			sideLeft,
			sideTop,
			sideWidth,
			sideHeight);
   }
#endif

   if (NULL != mStitch)
   {
       mStitch->start(mCapture,
			fps*2,
			pano2DWidth,
			pano2DHeight,
			configFilePath,
			enableOpenCL);
   }

   if (NULL != mCapture)
   {
       mCapture->start(fps);
   }
}

void Controller::stop()
{
    if (NULL != mCapture)
    {
         mCapture->stop();
         mCapture->closeDevice();
    }

    if (NULL != mStitch)
    {
        mStitch->stop();
    }
}

surround_image_t* Controller::dequeuePano2DImage()
{
    surround_image_t* tmp = NULL;
    if (NULL != mStitch)
    {
        tmp = mStitch->dequeuePano2DImage();
    }
    return tmp;
}

surround_image_t* Controller::dequeueSideImage(unsigned int channelIndex)
{
    surround_image_t* tmp = NULL;
    if (NULL != mStitch)
    {
        tmp = mStitch->dequeueSideImage(channelIndex);
    }
    return tmp;
}
