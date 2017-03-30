#include "capture4impl.h"
#include "capture4workerimpl.h"
#include "capture4workerv4l2impl.h"

Capture4Impl::Capture4Impl(QObject *parent) :
    QObject(parent),
    mCaptureWorker(NULL)
{
}

int Capture4Impl::openDevice()
{
    if (NULL == mCaptureWorker)
    {
#ifdef CAPTURE_ON_V4L2
        mCaptureWorker = new Capture4WorkerV4l2Impl(NULL, 4);
#else
        mCaptureWorker = new Capture4WorkerImpl(NULL, 4);
#endif
        mCaptureWorker->openDevice();
        connect(&mVideoCaptureTimer, SIGNAL(timeout()), mCaptureWorker, SLOT(onCapture()));
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

int Capture4Impl::start(VIDEO_FPS fps)
{
    mFPS = fps;
    mVideoCaptureTimer.start(1000/mFPS);

    //capture speed up, update speed down
    if (NULL != mCaptureWorker)
    {
        mCaptureWorker->moveToThread(&mCaptureThread);
        mCaptureThread.start(QThread::TimeCriticalPriority);
    }

    return 0;
}

int Capture4Impl::stop()
{
    mVideoCaptureTimer.stop();
    mCaptureThread.quit();

    return 0;
}

surround_image4_t* Capture4Impl::popOneFrame()
{
    surround_image4_t* pFrame = NULL;
    if (NULL != mCaptureWorker)
    {
        pFrame = mCaptureWorker->popOneFrame();
    }
    return pFrame;
}
