#include "stitchworker.h"
#include "stitch_algorithm.h"
#include "ICapture.h"
#include <opencv/cv.h>
#include <string>

using namespace std;

using namespace cv;

StitchWorker::StitchWorker()
{
    mCurChannelIndex = VIDEO_CHANNEL_FRONT;
    mLastTimestamp = 0.0;
    mEnableOpenCL = false;

    pthread_mutex_init(&mOutputPano2DImageMutex, NULL);
    pthread_mutex_init(&mOutputSideImageMutex, NULL);
}

StitchWorker::~StitchWorker()
{

}

void StitchWorker::init(ICapture *capture,	
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		char* configFilePath,
		bool enableOpenCL)
{
    if (NULL == capture)
    {
        return;
    }

    mPano2DWidth = pano2DWidth;
    mPano2DHeight = pano2DHeight;
    mEnableOpenCL = enableOpenCL;

    stitching_init(configFilePath, mStitchMap, mMask, mEnableOpenCL);

    if (mEnableOpenCL)
    {
        std::cout <<"mStitchMap rows:"<< mStitchMap.rows  << " cols:" << mStitchMap.cols  << " channel:" << mStitchMap.channels() << std::endl;
        std::cout <<"mMask rows:"<< mMask.rows  << " cols:" << mMask.cols  << " channel:" << mMask.channels() << std::endl;
        std::cout <<"mPano2DHeight:"<< mPano2DHeight  << " mPano2DWidth:" << mPano2DWidth  << std::endl;
        int mapX[mPano2DHeight][mPano2DWidth];
        int mapY[mPano2DHeight][mPano2DWidth];
        for (unsigned int i = 0; i < mPano2DHeight; i++)
        {
            for (unsigned int j = 0; j < mPano2DWidth; j++)
            {
                mapX[i][j] = mStitchMap.ptr<Point2f>(i)[j].x;
                mapY[i][j] = mStitchMap.ptr<Point2f>(i)[j].y;
            }
        }

        mStitchMapX = Mat(mPano2DHeight, mPano2DWidth, CV_32SC1, mapX);
        mStitchMapY = Mat(mPano2DHeight, mPano2DWidth, CV_32SC1, mapY);
        std::cout <<"mStitchMapX rows:"<< mStitchMapX.rows  << " cols:" << mStitchMapX.cols  << " channel:" << mStitchMapX.channels() << std::endl;
        std::cout <<"mStitchMapY rows:"<< mStitchMapY.rows  << " cols:" << mStitchMapY.cols  << " channel:" << mStitchMapY.channels() << std::endl;
    }

    mCapture = capture;
}

void StitchWorker::run()
{
    double timestamp = 0;

    surround_images_t* surroundImage = mCapture->popOneFrame();
    if (NULL == surroundImage)
    {
        return;
    }

    void* outSide = NULL;
    void* outPano2D = NULL;
#if DEBUG_STITCH
    int pano2d_size = 0;
    int side_size = 0;
    double end = 0.0;
#endif
    timestamp = surroundImage->timestamp;
    double elapsed = 0;
    double start = clock();
    elapsed = ((start - surroundImage->timestamp)/CLOCKS_PER_SEC);

#if DEBUG_STITCH
    double elapsed_to_last = 0;
    if (mLastTimestamp > 0.00001f)
    {
        elapsed_to_last = (start - mLastTimestamp)/CLOCKS_PER_SEC;
    }
    mLastTimestamp = start;
#endif

    if ((int)(elapsed*1000) < 500)
    {
        if (mEnableOpenCL)
        {
            stitching_cl(surroundImage->frame[VIDEO_CHANNEL_FRONT].data,
                         surroundImage->frame[VIDEO_CHANNEL_REAR].data,
                         surroundImage->frame[VIDEO_CHANNEL_LEFT].data,
                         surroundImage->frame[VIDEO_CHANNEL_RIGHT].data,
                         mStitchMapX,
                         mStitchMapY,
                         mMask,
                         &outPano2D,
                         mPano2DWidth,
                         mPano2DHeight,
                         &outSide,
                         surroundImage->frame[mCurChannelIndex].width,
                         surroundImage->frame[mCurChannelIndex].height,
                         mCurChannelIndex);
        }
        else
        {
            stitching(surroundImage->frame[VIDEO_CHANNEL_FRONT].data,
                      surroundImage->frame[VIDEO_CHANNEL_REAR].data,
                      surroundImage->frame[VIDEO_CHANNEL_LEFT].data,
                      surroundImage->frame[VIDEO_CHANNEL_RIGHT].data,
                      mStitchMap,
                      mMask,
                      &outPano2D,
                      mPano2DWidth,
                      mPano2DHeight,
                      &outSide,
                      surroundImage->frame[mCurChannelIndex].width,
                      surroundImage->frame[mCurChannelIndex].height,
                      mCurChannelIndex);
        }
    }
#if DEBUG_STITCH
    end = clock();
#endif

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        if (NULL != surroundImage->frame[i].data)
        {
            delete (cv::Mat*)surroundImage->frame[i].data;
        }
    }

    delete surroundImage;
    surroundImage = NULL;

    if (NULL != outPano2D)
    {
        surround_image_t* tmp = new surround_image_t();
        tmp->frame.data = outPano2D;
        tmp->timestamp = timestamp;
        pthread_mutex_lock(&mOutputPano2DImageMutex);
        mOutputPano2DImageQueue.push(tmp);
#if DEBUG_STITCH
        pano2d_size = mOutputPano2DImageQueue.size();
#endif
        pthread_mutex_unlock(&mOutputPano2DImageMutex);
    }

    if (NULL != outSide)
    {
        surround_image_t* tmp = new surround_image_t();
        tmp->frame.data = outSide;
        tmp->timestamp = timestamp;
        pthread_mutex_lock(&mOutputSideImageMutex);
        mOutputSideImageQueue.push(tmp);
#if DEBUG_STITCH
        side_size = mOutputSideImageQueue.size();
#endif
        pthread_mutex_unlock(&mOutputSideImageMutex);
    }

#if DEBUG_STITCH

    std::cout << "StitchWorke::run"
	     << " thread id:" << getTID()
             <<", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << elapsed
            << ", stitch:" << (end-start)/CLOCKS_PER_SEC
            << ", pano2d_size:" << pano2d_size
            << ", channel:" << mCurChannelIndex
            << ", side_size:" << side_size
	    << std::endl;
#endif

}

surround_image_t* StitchWorker::dequeuePano2DImage()
{
    surround_image_t* image = NULL;
    {
        pthread_mutex_lock(&mOutputPano2DImageMutex);;
        if (mOutputPano2DImageQueue.size() > 0)
        {
            image = mOutputPano2DImageQueue.front();
	    mOutputPano2DImageQueue.pop();
        }
	pthread_mutex_unlock(&mOutputPano2DImageMutex);
    }
    return image;
}

surround_image_t* StitchWorker::dequeueSideImage(unsigned int  channel)
{
    if (mCurChannelIndex != channel
            && channel < VIDEO_CHANNEL_SIZE)
    {
        mCurChannelIndex = channel;
    }

    surround_image_t* image = NULL;
    {
        pthread_mutex_lock(&mOutputSideImageMutex);
        if (mOutputSideImageQueue.size() > 0)
        {
            image = mOutputSideImageQueue.front();
	    mOutputSideImageQueue.pop();
        }
	pthread_mutex_unlock(&mOutputSideImageMutex);
    }
    return image;
}
