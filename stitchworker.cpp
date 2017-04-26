#include "stitchworker.h"
#include "stitch_algorithm.h"
#include "ICapture.h"
#include "settings.h"
#include <QDebug>
#include <opencv/cv.h>
#include <string>

using namespace std;

using namespace cv;

StitchWorker::StitchWorker() :
    mVideoChannel(VIDEO_CHANNEL_FRONT)
{
    mLastTimestamp = 0.0;
    mEnableOpenCL = false;
}

void StitchWorker::start(ICapture *capture)
{
    if (NULL == capture)
    {
        return;
    }

    mPano2DWidth = Settings::getInstant()->mPano2DWidth;
    mPano2DHeight = Settings::getInstant()->mPano2DHeight;
    mEnableOpenCL = (Settings::getInstant()->mEnableOpenCL == 1);

    QString path = Settings::getInstant()->getApplicationPath() + "/PanoConfig.bin";
    stitching_init(path.toStdString(), mStitchMap, mMask, mEnableOpenCL);

    if (mEnableOpenCL)
    {
        qDebug() <<"mStitchMap rows:"<< mStitchMap.rows  << " cols:" << mStitchMap.cols  << " channel:" << mStitchMap.channels() << endl;
        qDebug() <<"mMask rows:"<< mMask.rows  << " cols:" << mMask.cols  << " channel:" << mMask.channels() << endl;
        qDebug() <<"mPano2DHeight:"<< mPano2DHeight  << " mPano2DWidth:" << mPano2DWidth  << endl;
        int mapX[mPano2DHeight][mPano2DWidth];
        int mapY[mPano2DHeight][mPano2DWidth];
        for (int i = 0; i < mPano2DHeight; i++)
        {
            for (int j = 0; j < mPano2DWidth; j++)
            {
                mapX[i][j] = mStitchMap.ptr<Point2f>(i)[j].x;
                mapY[i][j] = mStitchMap.ptr<Point2f>(i)[j].y;
            }
        }

        mStitchMapX = Mat(mPano2DHeight, mPano2DWidth, CV_32SC1, mapX);
        mStitchMapY = Mat(mPano2DHeight, mPano2DWidth, CV_32SC1, mapY);
        qDebug() <<"mStitchMapX rows:"<< mStitchMapX.rows  << " cols:" << mStitchMapX.cols  << " channel:" << mStitchMapX.channels() << endl;
        qDebug() <<"mStitchMapY rows:"<< mStitchMapY.rows  << " cols:" << mStitchMapY.cols  << " channel:" << mStitchMapY.channels() << endl;
    }

    mCapture = capture;
}

void StitchWorker::stop()
{

}

void StitchWorker::onStitch()
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
    int elapsed = 0;
    double start = (double)clock();
    elapsed = (int)(start - surroundImage->timestamp)/1000;

    if (elapsed < 1500)
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
                         surroundImage->frame[mVideoChannel].width,
                         surroundImage->frame[mVideoChannel].height,
                         mVideoChannel);
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
                      surroundImage->frame[mVideoChannel].width,
                      surroundImage->frame[mVideoChannel].height,
                      mVideoChannel);
        }
    }
#if DEBUG_STITCH
    end = (double)clock();
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
        QMutexLocker locker(&mOutputFullImageMutex);
        mOutputFullImageQueue.append(tmp);
#if DEBUG_STITCH
        pano2d_size = mOutputFullImageQueue.size();
#endif
    }

    if (NULL != outSide)
    {
        surround_image_t* tmp = new surround_image_t();
        tmp->frame.data = outSide;
        tmp->timestamp = timestamp;
        QMutexLocker locker(&mOutputSmallImageMutex);
        mOutputSmallImageQueue.append(tmp);
#if DEBUG_STITCH
        side_size = mOutputSmallImageQueue.size();
#endif
    }

#if DEBUG_STITCH
    int elapsed_to_last = 0;
    if (qAbs(mLastTimestamp) > 0.00001f)
    {
        elapsed_to_last = (int)(start - mLastTimestamp)/1000;
    }
    mLastTimestamp = start;

    qDebug() << "StitchWorke::run"
             <<" elapsed to last time:" << elapsed_to_last
            << " elapsed to capture:" << elapsed
            << ", stitch:" << (int)(end-start)/1000
            << ", pano2d_size:" << pano2d_size
            << ", channel" << mVideoChannel
            << ", side_size:" << side_size;
#endif

}

surround_image_t* StitchWorker::dequeueFullImage()
{
    surround_image_t* image = NULL;
    {
        QMutexLocker locker(&mOutputFullImageMutex);
        if (mOutputFullImageQueue.size() > 0)
        {
            image = mOutputFullImageQueue.dequeue();
        }
    }
    return image;
}

surround_image_t* StitchWorker::dequeueSmallImage(VIDEO_CHANNEL channel)
{
    if (mVideoChannel != channel
            && channel < VIDEO_CHANNEL_SIZE)
    {
        mVideoChannel = channel;
    }

    surround_image_t* image = NULL;
    {
        QMutexLocker locker(&mOutputSmallImageMutex);
        if (mOutputSmallImageQueue.size() > 0)
        {
            image = mOutputSmallImageQueue.dequeue();
        }
    }
    return image;
}
