#include "capture1impl.h"
#include "capture1workerimpl.h"
#include "capture1workerv4l2impl.h"

Capture1Impl::Capture1Impl(QObject *parent) :
    QObject(parent)
{
    memset(mCaptureWorker, 0, sizeof(mCaptureWorker));
}

int Capture1Impl::openDevice()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
#ifdef CAPTURE_ON_V4L2
        mCaptureWorker[i] = new Capture1WorkerV4l2Impl(NULL, i);
#else
        mCaptureWorker[i] = new Capture1WorkerImpl(NULL, i);
#endif
        mCaptureWorker[i]->openDevice();
        connect(&mVideoCaptureTimer[i], SIGNAL(timeout()), mCaptureWorker[i], SLOT(onCapture()));
    }

    return 0;
}

int Capture1Impl::closeDevice()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
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

int Capture1Impl::start(VIDEO_FPS fps)
{
    mFPS = fps;

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mVideoCaptureTimer[i].start(1000/mFPS);

        //capture speed up, update speed down
        mCaptureWorker[i]->moveToThread(&mCaptureThread[i]);
        mCaptureThread[i].start();
    }

    return 0;
}

int Capture1Impl::stop()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mVideoCaptureTimer[i].stop();
        mCaptureThread[i].quit();
    }

    return 0;
}

surround_image4_t* Capture1Impl::popOneFrame()
{
    surround_image4_t* pFrame = NULL;
    bool isFullFrame = true;
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        if (mCaptureWorker[i]->getFrameCount() == 0)
        {
            isFullFrame = false;
        }
    }

    if (isFullFrame)
    {
        pFrame = new surround_image4_t();
        pFrame->timestamp = 0;
        for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            surround_image1_t* tmp = mCaptureWorker[i]->popOneFrame();
            if (NULL != tmp)
            {
                pFrame->image[i] = tmp->image;

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
