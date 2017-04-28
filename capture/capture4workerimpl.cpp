#include "capture4workerimpl.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

Capture4WorkerImpl::Capture4WorkerImpl() :
    Capture4WorkerBase()
{
    memset(mCaptureArray, 0, sizeof(mCaptureArray));
}

Capture4WorkerImpl::~Capture4WorkerImpl()
{

}

int Capture4WorkerImpl::openDevice(unsigned int channel[], unsigned int channelNum)
{
    mVideoChannelNum = channelNum <= VIDEO_CHANNEL_SIZE ? channelNum: VIDEO_CHANNEL_SIZE;
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
        unsigned int videoChannel = channel[i];
        mCaptureArray[i] = cvCreateCameraCapture(videoChannel);
        if (mCaptureArray[i] == NULL)
        {
            return -1;
        }
    }

    return 0;
}

void Capture4WorkerImpl::closeDevice()
{
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
        if (NULL != mCaptureArray[i])
        {
           cvReleaseCapture(&mCaptureArray[i]);
        }
    }
}

void Capture4WorkerImpl::run()
{
    double timestamp = (double)clock();
#if DEBUG_CAPTURE
    int size = 0;
    double elapsed = 0;
    if (mLastTimestamp > 0.00001f)
    {
        elapsed = (int)(timestamp - mLastTimestamp)/CLOCKS_PER_SEC;
    }
    mLastTimestamp = timestamp;
#endif

    surround_images_t* surroundImage = new surround_images_t();
    surroundImage->timestamp = timestamp;
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
        IplImage* pIplImage = cvQueryFrame(mCaptureArray[i]);
        cv::Mat* image = new cv::Mat(pIplImage, true);
        surroundImage->frame[i].data = image;
    }

#if DEBUG_CAPTURE
    double end0 = (double)clock();
#endif

    {
        pthread_mutex_lock(&mMutexQueue);
        mSurroundImageQueue.push(surroundImage);
#if DEBUG_CAPTURE
        size = mSurroundImageQueue.size();
#endif
	pthread_mutex_unlock(&mMutexQueue);
    }

#if DEBUG_CAPTURE
    double end1 = (double)clock();
#endif

#if DEBUG_CAPTURE
    std::cout << "Capture4WorkerImpl::onCapture"
             << ", channel:" << mVideoChannelNum
             << ", size: "<< size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (end0-mLastTimestamp)/CLOCKS_PER_SEC
             << ", write:" << (end1-end0)/CLOCKS_PER_SEC
	     << std::endl;
#endif
}
