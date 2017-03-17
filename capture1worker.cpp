#include "capture1worker.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

Capture1Worker::Capture1Worker(QObject *parent, int videoChannel) :
    QObject(parent),
    mVideoChannel(videoChannel),
    mCapture(NULL)
{

    mDropFrameCount = 10;
    mLastTimestamp = 0.0;
}

void Capture1Worker::openDevice()
{
    if (NULL == mCapture)
    {
        mCapture = cvCreateCameraCapture(mVideoChannel);
    }
}

void Capture1Worker::closeDevice()
{
    if (NULL != mCapture)
    {
        cvReleaseCapture(&mCapture);
        mCapture = NULL;
    }
}

void Capture1Worker::onCapture()
{
    if (mDropFrameCount-- > 0)
    {
        cvQueryFrame(mCapture);
        return;
    }

    surround_image1_t* surroundImage = new surround_image1_t();
    surroundImage->timestamp = (double)clock();

#if DEBUG
    int size = 0;
    int elapsed = 0;
    if (mLastTimestamp > 0)
    {
        elapsed = (int)(surroundImage->timestamp - mLastTimestamp)/1000;
    }
    mLastTimestamp = surroundImage->timestamp;
#endif

    surroundImage->image = cvQueryFrame(mCapture);

#if DEBUG
    double end0 = (double)clock();
#endif

    {
        QMutexLocker locker(&mMutex);
        mSurroundImageQueue.append(surroundImage);
#if DEBUG
        size = mSurroundImageQueue.size();
#endif
    }

#if DEBUG
    double end1 = (double)clock();
#endif

#if DEBUG
    qDebug() << "Capture1Worker::onCapture"
             << ", channel:" << mVideoChannel
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (int)(end0-mLastTimestamp)/1000
             << ", write:" << (int)(end1-end0)/1000;
#endif
}

int Capture1Worker::getFrameCount()
{
    QMutexLocker locker(&mMutex);
    return mSurroundImageQueue.size();
}

surround_image1_t* Capture1Worker::popOneFrame()
{
    struct surround_image1_t* surroundImage = NULL;

    {
        QMutexLocker locker(&mMutex);
        if (mSurroundImageQueue.size() > 0)
        {
            surroundImage = mSurroundImageQueue.dequeue();
        }
    }

    return surroundImage;
}
