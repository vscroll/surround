#include "capture1workerimpl.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QDebug>

Capture1WorkerImpl::Capture1WorkerImpl(QObject *parent, int videoChannel) :
    Capture1WorkerBase(parent, videoChannel),
    mCapture(NULL)
{

    mDropFrameCount = 10;
}

int Capture1WorkerImpl::openDevice()
{
    if (NULL == mCapture)
    {
        mCapture = cvCreateCameraCapture(mVideoChannel);
    }

    return mCapture == NULL ? -1:0;
}

void Capture1WorkerImpl::closeDevice()
{
    if (NULL != mCapture)
    {
        cvReleaseCapture(&mCapture);
        mCapture = NULL;
    }
}

void Capture1WorkerImpl::onCapture()
{

    mMutexDrop.lock();
    int count = mDropFrameCount--;
    mMutexDrop.unlock();

    if (count >= 0)
    {
        QMutexLocker locker(&mMutexCapture);
        cvQueryFrame(mCapture);
        return;
    }

    double timestamp = (double)clock();
    IplImage *tmpImage = NULL;
#if DEBUG_CAPTURE
    int size = 0;
    double elapsed = 0;
    if (qAbs(mLastTimestamp) > 0.00001f)
    {
        elapsed = (int)(timestamp - mLastTimestamp)/CLOCKS_PER_SEC;
    }
    mLastTimestamp = timestamp;
#endif

    {
        QMutexLocker locker(&mMutexCapture);
        tmpImage = cvQueryFrame(mCapture);
    }

#if DEBUG_CAPTURE
    double end0 = (double)clock();
#endif

    if (NULL != tmpImage)
    {
        cv::Mat* image = new cv::Mat(tmpImage, true);
        surround_image_t* surroundImage = new surround_image_t();
        surroundImage->timestamp = timestamp;
        surroundImage->frame.data = image;
        QMutexLocker locker(&mMutexQueue);
        mSurroundImageQueue.append(surroundImage);
#if DEBUG_CAPTURE
        size = mSurroundImageQueue.size();
#endif
    }

#if DEBUG_CAPTURE
    double end1 = (double)clock();
#endif

#if DEBUG_CAPTURE
    qDebug() << "Capture1WorkerImpl::onCapture"
             << " channel:" << mVideoChannel
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (end0-mLastTimestamp)/CLOCKS_PER_SEC
             << ", write:" << (end1-end0)/CLOCKS_PER_SEC;
#endif
}
