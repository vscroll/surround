#include "controller.h"
#include "common.h"
#include "ICapture.h"
#include "captureimpl.h"
#include "IPano.h"
#include "panoimpl.h"

Controller::Controller()
{
    mCapture = NULL;
    mPano = NULL;
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
    if (NULL == mPano)
    {
        mPano = new PanoImpl();
    }

    mPano->init(inWidth, inHeight, inPixfmt, panoWidth, panoHeight, panoPixfmt, algoCfgFilePath, enableOpenCL);
}

void Controller::initSideImageModule(unsigned int width, unsigned int height, unsigned int pixfmt)
{

}

void Controller::uninitModules()
{
    if (NULL != mCapture)
    {
        delete mCapture;
        mCapture = NULL;
    }

    if (NULL != mPano)
    {
        delete mPano;
        mPano = NULL;
    }
}

void Controller::startModules(unsigned int fps)
{
   if (NULL != mCapture)
   {
        mCapture->start(fps);
   }

   if (NULL != mPano)
   {
        mPano->start(fps);
   }
}

void Controller::stopModules()
{
    if (NULL != mCapture)
    {
        mCapture->stop();
        mCapture->closeDevice();
    }

    if (NULL != mPano)
    {
        mPano->stop();
    }
}

void Controller::startLoop(unsigned int freq)
{
    start(1000/freq);
}

void Controller::run()
{
    if (NULL == mCapture
        || NULL == mPano)
    {
        return;
    }

    surround_images_t* surroundImages = mCapture->popOneFrame();
    if (NULL == surroundImages)
    {
        return;
    }

    mPano->queueImages(surroundImages);
    surround_image_t* surroundImage = mPano->dequeuePanoImage();
}
