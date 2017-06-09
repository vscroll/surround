#ifndef SOURCESHMWRITEWORKER_H
#define SOURCESHMWRITEWORKER_H

#include "wrap_thread.h"

class ICapture;
class ImageSHM;
class SourceSHMWriteWorker : public WrapThread
{
public:
    SourceSHMWriteWorker(ICapture* capture, unsigned int channelIndex);
    virtual ~SourceSHMWriteWorker();

public:
    virtual void run();

private:
    //pthread_mutex_t mMutex;

    unsigned int mChannelIndex;
    ICapture* mCapture;
    ImageSHM* mImageSHM;
    clock_t mLastCallTime;
};

#endif // ALLSOURCESSHMWRITEWORKER_H
