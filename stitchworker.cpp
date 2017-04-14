#include "stitchworker.h"
#include "stitch_algorithm.h"
#include "ICapture.h"
#include "settings.h"
#include <QDebug>

StitchWorker::StitchWorker() :
    mIsRunning(true),
    mVideoChannel(VIDEO_CHANNEL_FRONT),
    mFreq(10)
{

}

void StitchWorker::start(ICapture *capture)
{
    if (NULL == capture)
    {
        return;
    }

    mFullWidth = Settings::getInstant()->mFullWidth;
    mFullHeight = Settings::getInstant()->mFullHeight;

    QString path = Settings::getInstant()->getApplicationPath() + "/PanoConfig.bin";
    stitching_init(path.toStdString(), mStitchMap, mMask);

    mCapture = capture;
    mIsRunning = true;
    QThread::start(QThread::TimeCriticalPriority);
}

void StitchWorker::stop()
{
    mIsRunning = false;
}

void StitchWorker::run()
{
    double timestamp = 0;
    while (true) {

        if (!mIsRunning)
        {
            usleep(mFreq);
            break;
        }

        surround_images_t* surroundImage = mCapture->popOneFrame();
        if (NULL == surroundImage)
        {
            continue;
        }

        void* outSmall = NULL;
        void* outFull = NULL;
#if DEBUG_STITCH
        int fullsize = 0;
        int smallsize = 0;
        double end = 0.0;
#endif
        timestamp = surroundImage->timestamp;
        int elapsed = 0;
        double start = (double)clock();
        elapsed = (int)(start - surroundImage->timestamp)/1000;
        if (elapsed < 1500)
        {
                stitching(surroundImage->frame[VIDEO_CHANNEL_FRONT].data,
                  surroundImage->frame[VIDEO_CHANNEL_REAR].data,
                  surroundImage->frame[VIDEO_CHANNEL_LEFT].data,
                  surroundImage->frame[VIDEO_CHANNEL_RIGHT].data,
                  mStitchMap,
                  mMask,
                  &outFull,
                  mFullWidth,
                  mFullHeight,
                  &outSmall,
                  mVideoChannel);
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

        if (NULL != outFull)
        {
            surround_image_t* tmp = new surround_image_t();
            tmp->frame.data = outFull;
            tmp->timestamp = timestamp;
            QMutexLocker locker(&mOutputFullImageMutex);
            mOutputFullImageQueue.append(tmp);
#if DEBUG_STITCH
            fullsize = mOutputFullImageQueue.size();
#endif
        }

        if (NULL != outSmall)
        {
            surround_image_t* tmp = new surround_image_t();
            tmp->frame.data = outSmall;
            tmp->timestamp = timestamp;
            QMutexLocker locker(&mOutputSmallImageMutex);
            mOutputSmallImageQueue.append(tmp);
#if DEBUG_STITCH
            smallsize = mOutputSmallImageQueue.size();
#endif
        }

#if DEBUG_STITCH
        qDebug() << "StitchWorke::run"
                 << " elapsed to capture:" << elapsed
                 << ", stitch:" << (int)(end-start)/1000
                 << ", fullsize:" << fullsize
                 << ", channel" << mVideoChannel
                 << ", smallsize:" << smallsize;
#endif

        usleep(mFreq);
    }
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
