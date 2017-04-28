#ifndef CAPTUREWORKERIMPL_H
#define CAPTUREWORKERIMPL_H

#include "common.h"
#include "capture4workerbase.h"

class CvCapture;
class Capture4WorkerImpl : public Capture4WorkerBase
{
public:
    Capture4WorkerImpl();
    virtual ~Capture4WorkerImpl();

public:
    virtual int openDevice(unsigned int channel[], unsigned int channelNum);
    virtual void closeDevice();
    virtual void run();
private:
    CvCapture *mCaptureArray[VIDEO_CHANNEL_SIZE];
};

#endif // CAPTUREWORKERIMPL_H
