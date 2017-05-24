#ifndef RENDERMARKWORKER_H
#define RENDERMARKWORKER_H

#include "renderbase.h"
#include "thread.h"

class RenderMarkWorker : public RenderBase, public Thread
{
public:
    RenderMarkWorker();
    virtual ~RenderMarkWorker();

    virtual int openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight);

    void setChannelMarkRect(unsigned int left,
		    unsigned int top,
		    unsigned int width,
		    unsigned int height);

    void getChannelMarkRect(unsigned int* left,
		    unsigned int* top,
		    unsigned int* width,
		    unsigned int* height);
public:
    virtual void run();

private:
    unsigned int mChannelMarkLeft;
    unsigned int mChannelMarkTop;
    unsigned int mChannelMarkWidth;
    unsigned int mChannelMarkHeight;

    unsigned int mFocusChannelIndex;
    unsigned int mUpdateChannelIndex;

    clock_t mLastCallTime;
};

#endif // RENDERMARKWORKER_H
