#include "capture4impl.h"
#include "capture4worker.h"

Capture4Impl::Capture4Impl(QObject *parent) :
    QObject(parent),
    mCaptureWorker(NULL)
{
}

int Capture4Impl::openDevice()
{
    if (NULL == mCaptureWorker)
    {
        mCaptureWorker = new Capture4Worker(NULL, 4);
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
        mCaptureThread.start();
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
