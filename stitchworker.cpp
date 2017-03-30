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


    stitching_init("Fish2Pano.bin", mStitchMaps);

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

        surround_image4_t* surroundImage = mCapture->popOneFrame();
        if (NULL == surroundImage)
        {
            continue;
        }

        void* outSmall = NULL;
        void* outFull = NULL;
#if DEBUG
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
                stitching(surroundImage->image[VIDEO_CHANNEL_FRONT],
                  surroundImage->image[VIDEO_CHANNEL_REAR],
                  surroundImage->image[VIDEO_CHANNEL_LEFT],
                  surroundImage->image[VIDEO_CHANNEL_RIGHT],
                  mStitchMaps,
                  &outFull,
                  &outSmall,
                  mVideoChannel);
         }
#if DEBUG
        end = (double)clock();
#endif
        for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            if (NULL != surroundImage->image[i])
            {
#if DATA_TYPE_IPLIMAGE
                cvReleaseImage((IplImage**)surroundImage->image[i]);
#else
                delete (cv::Mat*)surroundImage->image[i];
#endif
            }
        }

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
                 << " elapsed to capture:" << elapsed
                 << ", stitch:" << (int)(end-start)/1000
                 << ", fullsize:" << fullsize
                 << ", channel" << mVideoChannel
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
