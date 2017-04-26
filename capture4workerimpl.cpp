#include "capture4workerimpl.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "settings.h"
#include <QDebug>

Capture4WorkerImpl::Capture4WorkerImpl(QObject *parent, int videoChannelNum) :
    Capture4WorkerBase(parent, videoChannelNum)
{
    memset(mCaptureArray, 0, sizeof(mCaptureArray));
}

int Capture4WorkerImpl::openDevice()
{
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        int video_channel = Settings::getInstant()->mVideoChanel[i];
        mCaptureArray[i] = cvCreateCameraCapture(video_channel);
        if (mCaptureArray[i] == NULL)
        {
            return -1;
        }
    }

    return 0;
}

void Capture4WorkerImpl::closeDevice()
{
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        if (NULL != mCaptureArray[i])
        {
           cvReleaseCapture(&mCaptureArray[i]);
        }
    }
}

void Capture4WorkerImpl::onCapture()
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
#if DEBUG_CAPTURE
    int size = 0;
    double elapsed = 0;
    if (qAbs(mLastTimestamp) > 0.00001f)
    {
        elapsed = (int)(timestamp - mLastTimestamp)/CLOCKS_PER_SEC;
    }
    mLastTimestamp = timestamp;
#endif

    surround_images_t* surroundImage = new surround_images_t();
    surroundImage->timestamp = timestamp;
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        IplImage* pIplImage = cvQueryFrame(mCaptureArray[i]);
        cv::Mat* image = new cv::Mat(pIplImage, true);
        surroundImage->frame[i].data = image;
    }

#if DEBUG_CAPTURE
    double end0 = (double)clock();
#endif

    {
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
    qDebug() << "Capture4WorkerImpl::onCapture"
             << " channel:" << mVideoChannelNum
             << ", size: "<< size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (end0-mLastTimestamp)/CLOCKS_PER_SEC
             << ", write:" << (end1-end0)/CLOCKS_PER_SEC;
#endif
}
