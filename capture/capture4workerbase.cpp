#include "capture4workerbase.h"

Capture4WorkerBase::Capture4WorkerBase()
{
    pthread_mutex_init(&mMutexQueue, NULL); 
    mLastTimestamp = 0.0;
}

Capture4WorkerBase::~Capture4WorkerBase()
{

}

int Capture4WorkerBase::openDevice(unsigned int channel[], unsigned int channelNum)
{
    return -1;
}

void Capture4WorkerBase::closeDevice()
{

}

surround_images_t* Capture4WorkerBase::popOneFrame()
{

    struct surround_images_t* surroundImage = NULL;
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

unsigned int Capture4WorkerBase::getFrameCount()
{
    pthread_mutex_lock(&mMutexQueue);
    unsigned int size = mSurroundImageQueue.size();
    pthread_mutex_unlock(&mMutexQueue);
    return size;
}

void Capture4WorkerBase::getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{

}

int Capture4WorkerBase::getFPS(unsigned int* fps)
{
    return -1;
}
