#include "controller.h"
#include "ICapture.h"
#include "IStitch.h"
#include "stitchimpl.h"
#include "capture1impl.h"
#include "capture4impl.h"

Controller::Controller()
{
    mCapture = NULL;
    mStitch = NULL;
}

Controller::~Controller()
{

}

void Controller::init(unsigned int channel[], unsigned int channelNum)
{
    if (NULL == mCapture)
    {
#if CAPTURE_4_CHANNEL_ONCE
        mCapture = new Capture4Impl();
#else
        mCapture = new Capture1Impl();
#endif
        mCapture->openDevice(channel, channelNum);
    }

    if (NULL == mStitch)
    {
        mStitch = new StitchImpl();
    }
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
}

void Controller::start(unsigned int fps,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		char* configFilePath,
		bool enableOpenCL)
{
   if (NULL != mStitch)
   {
       mStitch->start(mCapture,
			200,
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
