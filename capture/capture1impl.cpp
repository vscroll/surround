#include "capture1impl.h"
#include "capture1workerimpl.h"
#include "capture1workerv4l2impl.h"

Capture1Impl::Capture1Impl()
{
    mChannelNum = 0;
    memset(mCaptureWorker, 0, sizeof(mCaptureWorker));
}

Capture1Impl::~Capture1Impl()
{

}

int Capture1Impl::openDevice(unsigned int channel[], unsigned int channelNum)
{
    if (channelNum > VIDEO_CHANNEL_SIZE)
    {
	mChannelNum = VIDEO_CHANNEL_SIZE;
    }

    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        unsigned int videoChannel = channel[i];
#ifdef CAPTURE_ON_V4L2
        mCaptureWorker[i] = new Capture1WorkerV4l2Impl();
#else
        mCaptureWorker[i] = new Capture1WorkerImpl();
#endif
        mCaptureWorker[i]->openDevice(videoChannel);
    }

    return 0;
}

int Capture1Impl::closeDevice()
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (NULL != mCaptureWorker[i])
        {
            mCaptureWorker[i]->closeDevice();
            delete mCaptureWorker[i];
            mCaptureWorker[i] = NULL;
        }
    }

    return 0;
}

int Capture1Impl::start(unsigned int fps)
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
	mCaptureWorker[i]->start(1000/fps);
    }

    return 0;
}

int Capture1Impl::stop()
{
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        mCaptureWorker[i]->stop();
    }

    return 0;
}

surround_images_t* Capture1Impl::popOneFrame()
{
    surround_images_t* pFrame = NULL;
    bool isFullFrame = true;
    for (unsigned int i = 0; i < mChannelNum; ++i)
    {
        if (mCaptureWorker[i]->getFrameCount() == 0)
        {
            isFullFrame = false;
        }
    }

    if (isFullFrame)
    {
        pFrame = new surround_images_t();
        pFrame->timestamp = 0;
        for (unsigned int i = 0; i < mChannelNum; ++i)
        {
            surround_image_t* tmp = mCaptureWorker[i]->popOneFrame();
            if (NULL != tmp)
            {
                pFrame->frame[i].data = tmp->frame.data;

                // get the earliest one
                if (pFrame->timestamp == 0
                    || pFrame->timestamp > tmp->timestamp)
                {
                    pFrame->timestamp = tmp->timestamp;
                }
                delete tmp;
            }
        }
    }

    return pFrame;
}
