#include "capture1workerimpl.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <ostream>

Capture1WorkerImpl::Capture1WorkerImpl() :
    Capture1WorkerBase()
{
    mCapture = NULL;
    pthread_mutex_init(&mMutexCapture, NULL);
}

int Capture1WorkerImpl::openDevice(unsigned int channel)
{
    mVideoChannel = channel;
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

void Capture1WorkerImpl::run()
{
    double timestamp = (double)clock();
    IplImage *tmpImage = NULL;
#if DEBUG_CAPTURE
    int size = 0;
    double elapsed = 0;
    if (mLastTimestamp > 0.00001f)
    {
        elapsed = (int)(timestamp - mLastTimestamp)/CLOCKS_PER_SEC;
    }
    mLastTimestamp = timestamp;
#endif

    {
        pthread_mutex_lock(&mMutexCapture);
        tmpImage = cvQueryFrame(mCapture);
        pthread_mutex_unlock(&mMutexCapture);
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
        pthread_mutex_lock(&mMutexCapture);
        mSurroundImageQueue.push(surroundImage);
#if DEBUG_CAPTURE
        size = mSurroundImageQueue.size();
#endif
        pthread_mutex_unlock(&mMutexCapture);
    }

#if DEBUG_CAPTURE
    double end1 = (double)clock();
#endif

#if DEBUG_CAPTURE
    std::cout << "Capture1WorkerImpl::onCapture"
             << ", channel:" << mVideoChannel
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (end0-mLastTimestamp)/CLOCKS_PER_SEC
             << ", write:" << (end1-end0)/CLOCKS_PER_SEC
	     << std::endl;
#endif
}
