#include "controller.h"
#include "common.h"
#include "ICapture.h"
#include "captureimpl.h"
#include "IPanoImage.h"
#include "panoimageimpl.h"
#include "imageshm.h"
#include <opencv/cv.h>

Controller::Controller()
{
    mCapture = NULL;
    mPanoImage = NULL;

    mCurChannelIndex = VIDEO_CHANNEL_FRONT;

    mSideSHM = NULL;
    mPanoSHM = NULL;
}

Controller::~Controller()
{
}

void Controller::initCaptureModule(unsigned int channel[], unsigned int channelNum,
		struct cap_sink_t sink[], struct cap_src_t source[])
{
    if (NULL == mCapture)
    {
        mCapture = new CaptureImpl();
    }
    mCapture->setCapCapacity(sink, source, channelNum);
    mCapture->openDevice(channel, channelNum);
}

void Controller::initPanoImageModule(unsigned int inWidth,
            unsigned int inHeight,
            unsigned int inPixfmt,
            unsigned int panoWidth,
            unsigned int panoHeight,
            unsigned int panoPixfmt,
            char* algoCfgFilePath,
            bool enableOpenCL)
{
    if (NULL == mPanoImage)
    {
        mPanoImage = new PanoImageImpl();
    }

    mPanoImage->init(inWidth, inHeight, inPixfmt, panoWidth, panoHeight, panoPixfmt, algoCfgFilePath, enableOpenCL);
}

void Controller::initSideImageModule(unsigned int curChannelIndex,
            unsigned int outWidth,
            unsigned int outHeight,
            unsigned int outPixfmt)
{
    if (curChannelIndex < VIDEO_CHANNEL_SIZE)
    {
        mCurChannelIndex = curChannelIndex;
    }
}

void Controller::uninitModules()
{
    if (NULL != mCapture)
    {
        delete mCapture;
        mCapture = NULL;
    }

    if (NULL != mPanoImage)
    {
        delete mPanoImage;
        mPanoImage = NULL;
    }
}

void Controller::startModules(unsigned int fps)
{
   if (NULL != mCapture)
   {
        mCapture->start(fps);
   }

   if (NULL != mPanoImage)
   {
        mPanoImage->start(fps);
   }
}

void Controller::stopModules()
{
    if (NULL != mCapture)
    {
        mCapture->stop();
        mCapture->closeDevice();
    }

    if (NULL != mPanoImage)
    {
        mPanoImage->stop();
    }
}

void Controller::startLoop(unsigned int freq)
{
    mSideSHM = new ImageSHM();
    mSideSHM->create((key_t)SHM_SIDE_ID, SHM_SIDE_SIZE);

    mPanoSHM = new ImageSHM();
    mPanoSHM->create((key_t)SHM_PANO2D_ID, SHM_PANO2D_SIZE);

    start(1000/freq);
}

void Controller::stopLoop()
{
    if (NULL != mSideSHM)
    {
        mSideSHM->destroy();
        delete mSideSHM;
        mSideSHM = NULL;
    }

    if (NULL != mPanoSHM)
    {
        mPanoSHM->destroy();
        delete mPanoSHM;
        mPanoSHM = NULL;
    }
}

void Controller::run()
{
    if (NULL == mCapture
        || NULL == mPanoImage)
    {
        return;
    }

    if (NULL == mSideSHM
        || NULL == mPanoSHM)
    {
        return;
    }

    surround_images_t* surroundImages = mCapture->popOneFrame();
    if (NULL == surroundImages)
    {
        return;
    }

    //Side
    surround_image_t* sideImage = &(surroundImages->frame[mCurChannelIndex]);
    if (NULL != sideImage)
    {
		struct image_shm_header_t header = {};
		header.width = sideImage->info.width;
		header.height = sideImage->info.height;
		header.pixfmt = sideImage->info.pixfmt;
		header.size = sideImage->info.size;
		header.timestamp = sideImage->timestamp;
        mSideSHM->writeImage(&header, (unsigned char*)sideImage->data, header.size);
    }

    //Pano
    mPanoImage->queueImages(surroundImages);
    surround_image_t* surroundImage = mPanoImage->dequeuePanoImage();
}
