#ifndef RENDERPANOWORKER_H
#define RENDERPANOWORKER_H

#include "renderbase.h"
#include "thread.h"

class IPanoImage;
class RenderPanoWorker : public RenderBase, public Thread
{
public:
    RenderPanoWorker();
    virtual ~RenderPanoWorker();

    void init(IPanoImage* panoImage);
public:
    virtual void run();

private:
    IPanoImage* mPanoImage;
};

#endif // RENDERPANOWORKER_H
