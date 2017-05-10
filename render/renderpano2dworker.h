#ifndef RENDERPANO2DWORKER_H
#define RENDERPANO2DWORKER_H

#include "renderbase.h"
#include "thread.h"

class IStitch;
class RenderDevice;
class RenderPano2DWorker : public RenderBase, public Thread
{
public:
    RenderPano2DWorker();
    virtual ~RenderPano2DWorker();

public:
    virtual void run();
};

#endif // RENDERPANO2DWORKER_H
