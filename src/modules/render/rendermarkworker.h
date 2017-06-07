#ifndef RENDERMARKWORKER_H
#define RENDERMARKWORKER_H

#include "renderbase.h"
#include "wrap_thread.h"

class RenderMarkWorker : public RenderBase, public WrapThread
{
public:
    RenderMarkWorker();
    virtual ~RenderMarkWorker();

    virtual int openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight);

    void setMarkRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height);

    void getMarkRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height);
public:
    virtual void run();

private:
    unsigned int mMarkLeft;
    unsigned int mMarkTop;
    unsigned int mMarkWidth;
    unsigned int mMarkHeight;

    unsigned int mFocusChannelIndex;
    unsigned int mUpdateChannelIndex;

    clock_t mLastCallTime;
};

#endif // RENDERMARKWORKER_H
