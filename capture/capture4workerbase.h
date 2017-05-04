#ifndef CAPTURE4WORKERBASE_H
#define CAPTURE4WORKERBASE_H

#include <queue>
#include "common.h"
#include "thread.h"

class Capture4WorkerBase: public Thread
{
public:
    Capture4WorkerBase();
    virtual ~Capture4WorkerBase();

    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual void closeDevice();
    virtual surround_images_t* popOneFrame();
    virtual unsigned int getFrameCount();
    virtual unsigned int getVideoChannelNum() { return mVideoChannelNum; }
    virtual void getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
protected:
    unsigned int mVideoChannelNum;
    unsigned int mChannel[VIDEO_CHANNEL_SIZE];

    pthread_mutex_t mMutexQueue;
    std::queue<surround_images_t*> mSurroundImageQueue;

    double mLastTimestamp;
};

#endif // CAPTURE4WORKERBASE_H
