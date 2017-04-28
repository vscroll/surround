#ifndef CAPTURE1WORKERIMPL_H
#define CAPTURE1WORKERIMPL_H

#include "common.h"
#include "capture1workerbase.h"
#include <pthread.h>

class CvCapture;
class Capture1WorkerImpl : public Capture1WorkerBase
{
public:
    Capture1WorkerImpl();

    virtual int openDevice(unsigned int channel);
    virtual void closeDevice();
    virtual void run();
private:
    CvCapture* mCapture;
    pthread_mutex_t mMutexCapture;
};

#endif // CAPTURE1WORKERIMPL_H
