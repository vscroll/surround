#include "capture4impl.h"
#include "capture4workerimpl.h"
#include "capture4workerv4l2impl.h"

Capture4Impl::Capture4Impl()
{
    mCaptureWorker = NULL;
}

Capture4Impl::~Capture4Impl()
{

}

int Capture4Impl::openDevice(unsigned int channel[], struct cap_info_t capInfo[], unsigned int channelNum)
{
    if (NULL == mCaptureWorker)
    {
#ifdef CAPTURE_ON_V4L2
        mCaptureWorker = new Capture4WorkerV4l2Impl();
#else
        mCaptureWorker = new Capture4WorkerImpl();
#endif
        mCaptureWorker->openDevice(channel, capInfo, channelNum);
    }

    return 0;
}

int Capture4Impl::closeDevice()
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->closeDevice();
        delete mCaptureWorker;
        mCaptureWorker = NULL;
    }

    return 0;
}

int Capture4Impl::start(unsigned int fps)
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->start(1000/fps);
    }

    return 0;
}

int Capture4Impl::stop()
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->stop();
    }

    return 0;
}

void Capture4Impl::getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->getResolution(channelIndex, width, height);
    }
}

int Capture4Impl::getFPS(unsigned int* fps)
{
    if (NULL != mCaptureWorker)
    {
	return mCaptureWorker->getFPS(fps);
    }

    return -1;
}

surround_images_t* Capture4Impl::popOneFrame()
{
    surround_images_t* pFrame = NULL;
    if (NULL != mCaptureWorker)
    {
        pFrame = mCaptureWorker->popOneFrame();
    }
    return pFrame;
}
