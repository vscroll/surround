#ifndef CAPTURE1WORKERBASE_H
#define CAPTURE1WORKERBASE_H

#include "common.h"
#include <queue>
#include <opencv/highgui.h>
#include "thread.h"

class Capture1WorkerBase: public Thread
{
public:
    Capture1WorkerBase();
    virtual ~Capture1WorkerBase();

    virtual int openDevice(unsigned int channel);
    virtual void closeDevice();
    virtual surround_image_t* popOneFrame();
    virtual unsigned int getFrameCount();
    virtual void run();

protected:
    void write2File(IplImage* image);

protected:
    unsigned int mVideoChannel;
    std::queue<surround_image_t*> mSurroundImageQueue;
    pthread_mutex_t mMutexQueue;

    int mIPUFd;

    double mLastTimestamp;
    pthread_mutex_t mMutexFile;
};

#endif // CAPTURE1WORKERBASE_H
