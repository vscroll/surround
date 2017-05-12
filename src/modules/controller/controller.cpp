#include "controller.h"
#include "common.h"
#include "ICapture.h"
#include "captureimpl.h"

Controller::Controller()
{
    mCapture = NULL;
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

void Controller::initPanoImageModule(unsigned int width, unsigned int height, unsigned int pixfmt,
        char* algoCfgFilePath, bool enableOpenCL)
{

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
}

void Controller::startModules(unsigned int fps)
{
   if (NULL != mCapture)
   {
        mCapture->start(fps);
   }
}

void Controller::stopModules()
{
    if (NULL != mCapture)
    {
        mCapture->stop();
        mCapture->closeDevice();
    }
}

void Controller::run()
{

}
