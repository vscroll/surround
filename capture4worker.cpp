#include "capture4worker.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

Capture4Worker::Capture4Worker(QObject *parent, int videoChannelNum) :
    QObject(parent),
    mVideoChannelNum(videoChannelNum < VIDEO_CHANNEL_SIZE ? videoChannelNum : VIDEO_CHANNEL_SIZE)
{
    memset(mCaptureArray, 0, sizeof(mCaptureArray));

    mDropFrameCount = 10;
    mLastTimestamp = 0.0;
}

void Capture4Worker::openDevice()
{
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        mCaptureArray[i] = cvCreateCameraCapture(i);
    }
}

void Capture4Worker::closeDevice()
{
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        if (NULL != mCaptureArray[i])
        {
           cvReleaseCapture(&mCaptureArray[i]);
        }
    }
}

void Capture4Worker::onCapture()
{
    if (mDropFrameCount-- > 0)
    {
        for (int i = 0; i < mVideoChannelNum; ++i)
        {
            cvQueryFrame(mCaptureArray[i]);
        }
        return;
    }

    double timestamp = (double)clock();
#if DEBUG
    int size = 0;
    int elapsed = 0;
    if (qAbs(mLastTimestamp) > 0.00001f)
    {
        elapsed = (int)(timestamp - mLastTimestamp)/1000;
    }
    mLastTimestamp = timestamp;
#endif

    surround_image4_t* surroundImage = new surround_image4_t();
    surroundImage->timestamp = timestamp;
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        surroundImage->image[i] = cvQueryFrame(mCaptureArray[i]);
    }

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
    qDebug() << "Capture4Worker::onCapture"
             << ", channel:" << mVideoChannelNum
             << ", size: "<< size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (int)(end0-mLastTimestamp)/1000
             << ", write:" << (int)(end1-end0)/1000;
#endif
}

surround_image4_t* Capture4Worker::popOneFrame()
{
    struct surround_image4_t* surroundImage = NULL;

    {
        QMutexLocker locker(&mMutex);
        if (mSurroundImageQueue.size() > 0)
        {
            surroundImage = mSurroundImageQueue.dequeue();
        }
    }

    return surroundImage;
}
