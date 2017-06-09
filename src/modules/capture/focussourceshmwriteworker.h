#ifndef FOCUSSOURCESHMWRITEWORKER_H
#define FOCUSSOURCESHMWRITEWORKER_H

#include "wrap_thread.h"

class ICapture;
class ImageSHM;
class FocusSourceSHMWriteWorker : public WrapThread
{
public:
    FocusSourceSHMWriteWorker(ICapture* capture);
    virtual ~FocusSourceSHMWriteWorker();

public:
    virtual void run();

private:
    ICapture* mCapture;
    ImageSHM* mImageSHM;
};

#endif // FOCUSSOURCESHMWRITEWORKER_H
