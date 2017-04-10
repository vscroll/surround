#include "capture4workerbase.h"

Capture4WorkerBase::Capture4WorkerBase(QObject *parent, int videoChannelNum) :
    QObject(parent),
    mVideoChannelNum(videoChannelNum < VIDEO_CHANNEL_SIZE ? videoChannelNum : VIDEO_CHANNEL_SIZE)
{
    mDropFrameCount = 10;
    mLastTimestamp = 0.0;
}

int Capture4WorkerBase::openDevice()
{
    return -1;
}

void Capture4WorkerBase::closeDevice()
{

}

void Capture4WorkerBase::onCapture()
{

}

surround_image4_t* Capture4WorkerBase::popOneFrame()
{

    struct surround_image4_t* surroundImage = NULL;
    {
        QMutexLocker locker(&mMutexQueue);
        if (mSurroundImageQueue.size() > 0)
        {
            surroundImage = mSurroundImageQueue.dequeue();
        }
    }

    return surroundImage;
}

int Capture4WorkerBase::getFrameCount()
{
    QMutexLocker locker(&mMutexQueue);
    return mSurroundImageQueue.size();
}
