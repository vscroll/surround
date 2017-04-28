#include "capture1workerbase.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>

Capture1WorkerBase::Capture1WorkerBase()
{
    mLastTimestamp = 0.0;
    mIPUFd = -1;

    pthread_mutex_init(&mMutexQueue, NULL);
    pthread_mutex_init(&mMutexFile, NULL);
}

Capture1WorkerBase::~Capture1WorkerBase()
{

}

int Capture1WorkerBase::openDevice(unsigned int channel)
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

void Capture1WorkerBase::run()
{

}

surround_image_t* Capture1WorkerBase::popOneFrame()
{

    struct surround_image_t* surroundImage = NULL;

    {
        pthread_mutex_lock(&mMutexQueue);
        if (mSurroundImageQueue.size() > 0)
        {
            surroundImage = mSurroundImageQueue.front();
	    mSurroundImageQueue.pop();
        }
	pthread_mutex_unlock(&mMutexQueue);
    }

    return surroundImage;
}

unsigned int Capture1WorkerBase::getFrameCount()
{
    pthread_mutex_lock(&mMutexQueue);
    unsigned int size = mSurroundImageQueue.size();
    pthread_mutex_unlock(&mMutexQueue);
    return size;
}

void Capture1WorkerBase::write2File(IplImage* image)
{
    pthread_mutex_lock(&mMutexFile);
    Util::write2File(mVideoChannel, image);
    pthread_mutex_unlock(&mMutexFile);
}
