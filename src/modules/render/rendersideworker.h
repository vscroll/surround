#ifndef RENDERSIDEWORKER_H
#define RENDERSIDEWORKER_H

#include "renderbase.h"
#include "thread.h"

class ICapture;
class ImageSHM;
class RenderSideWorker : public RenderBase, public Thread
{
public:
    RenderSideWorker();
    virtual ~RenderSideWorker();

    void init(ICapture* capture);
public:
    virtual void run();

private:
    ICapture* mCapture;
    ImageSHM* mSideSHM;

    clock_t mLastCallTime;
};

#endif // RENDERSIDEWORKER_H
