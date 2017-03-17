#include "stitchworker.h"
#include "stitch_algorithm.h"
#include "ICapture.h"

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

    mCapture = capture;
    mIsRunning = true;
    QThread::start();
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

        surround_image4_t* surroundImage = mCapture->popOneFrame();
        if (NULL == surroundImage)
        {
            continue;
        }

        IplImage* outSmall = NULL;
        IplImage* outFull = NULL;
#if DEBUG
        int elapsed = 0;
        int fullsize = 0;
        int smallsize = 0;
        double start = 0.0;
        double end = 0.0;
        start = (double)clock();
        elapsed = (int)(start - surroundImage->timestamp)/1000;
#endif
        timestamp = surroundImage->timestamp;
        stitching((IplImage*)surroundImage->image[VIDEO_CHANNEL_FRONT],
                  (IplImage*)surroundImage->image[VIDEO_CHANNEL_REAR],
                  (IplImage*)surroundImage->image[VIDEO_CHANNEL_LEFT],
                  (IplImage*)surroundImage->image[VIDEO_CHANNEL_RIGHT],
                  &outFull,
                  &outSmall,
                  mVideoChannel);
#if DEBUG
        end = (double)clock();
#endif

        delete surroundImage;
        surroundImage = NULL;

        if (NULL != outFull)
        {
            surround_image1_t* tmp = new surround_image1_t();
            tmp->image = outFull;
            tmp->timestamp = timestamp;
            QMutexLocker locker(&mOutputFullImageMutex);
            mOutputFullImageQueue.append(tmp);
#if DEBUG
            fullsize = mOutputFullImageQueue.size();
#endif
        }

        if (NULL != outSmall)
        {
            surround_image1_t* tmp = new surround_image1_t();
            tmp->image = outSmall;
            tmp->timestamp = timestamp;
            QMutexLocker locker(&mOutputSmallImageMutex);
            mOutputSmallImageQueue.append(tmp);
#if DEBUG
            smallsize = mOutputSmallImageQueue.size();
#endif
        }

#if DEBUG
        qDebug() << "StitchWorke::run"
                 << ", elapsed to capture:" << elapsed
                 << ", stitch:" << (int)(end-start)/1000
                 << ", fullsize:" << fullsize
                 << ", smallsize:" << smallsize;
#endif

        usleep(mFreq);
    }
}

surround_image1_t* StitchWorker::dequeueFullImage()
{
    surround_image1_t* image = NULL;
    {
        QMutexLocker locker(&mOutputFullImageMutex);
        if (mOutputFullImageQueue.size() > 0)
        {
            image = mOutputFullImageQueue.dequeue();
        }
    }
    return image;
}

surround_image1_t* StitchWorker::dequeueSmallImage(VIDEO_CHANNEL channel)
{
    if (mVideoChannel != channel
            && channel < VIDEO_CHANNEL_SIZE)
    {
        mVideoChannel = channel;
    }

    surround_image1_t* image = NULL;
    {
        QMutexLocker locker(&mOutputSmallImageMutex);
        if (mOutputSmallImageQueue.size() > 0)
        {
            image = mOutputSmallImageQueue.dequeue();
        }
    }
    return image;
}
