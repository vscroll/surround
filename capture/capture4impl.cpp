#include "capture4impl.h"
#include "capture4workerimpl.h"
#include "capture4workerv4l2impl.h"

Capture4Impl::Capture4Impl()
{
#ifdef CAPTURE_ON_V4L2
    mCaptureWorker = new Capture4WorkerV4l2Impl();
#else
    mCaptureWorker = new Capture4WorkerImpl();
#endif
}

Capture4Impl::~Capture4Impl()
{

}

void Capture4Impl::setCapCapacity(struct cap_sink_t sink[], struct cap_src_t sideSrc[], struct cap_src_t panoSrc[], unsigned int channelNum)
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->setCapCapacity(sink, sideSrc, panoSrc, channelNum);
    }
}

int Capture4Impl::openDevice(unsigned int channel[], unsigned int channelNum)
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->openDevice(channel, channelNum);
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

void Capture4Impl::getSideResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->getSideResolution(channelIndex, width, height);
    }
}

void Capture4Impl::getPanoResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->getPanoResolution(channelIndex, width, height);
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
