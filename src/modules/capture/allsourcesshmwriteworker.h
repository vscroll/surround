#ifndef ALLSOURCESSHMWRITEWORKER_H
#define ALLSOURCESSHMWRITEWORKER_H

#include "wrap_thread.h"

class ICapture;
class ImageSHM;
class AllSourcesSHMWriteWorker : public WrapThread
{
public:
    AllSourcesSHMWriteWorker(ICapture* capture);
    virtual ~AllSourcesSHMWriteWorker();

public:
    virtual void run();

private:
    ICapture* mCapture;
    ImageSHM* mImageSHM;
    clock_t mLastCallTime;
};

#endif // ALLSOURCESSHMWRITEWORKER_H
