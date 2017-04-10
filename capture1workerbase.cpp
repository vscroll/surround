#include "capture1workerbase.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>

Capture1WorkerBase::Capture1WorkerBase(QObject *parent, int videoChannel) :
    QObject(parent),
    mVideoChannel(videoChannel),
    mLastTimestamp(0.0)
{
    mIPUFd = -1;
}

int Capture1WorkerBase::openDevice()
{
    mIPUFd = open("/dev/mxc_ipu", O_RDWR, 0);
    return mIPUFd;
}

void Capture1WorkerBase::closeDevice()
{
    if (mIPUFd > 0)
    {
        close(mIPUFd);
        mIPUFd = -1;
    }
}

void Capture1WorkerBase::onCapture()
{

}

surround_image1_t* Capture1WorkerBase::popOneFrame()
{

    struct surround_image1_t* surroundImage = NULL;

    {
        QMutexLocker locker(&mMutexQueue);
        if (mSurroundImageQueue.size() > 0)
        {
            surroundImage = mSurroundImageQueue.dequeue();
        }
    }

    return surroundImage;
}

int Capture1WorkerBase::getFrameCount()
{
    QMutexLocker locker(&mMutexQueue);
    return mSurroundImageQueue.size();
}

void Capture1WorkerBase::write2File(IplImage* image)
{
    mMutexFile.lock();
    Util::write2File(mVideoChannel, image);
    mMutexFile.unlock();
}
