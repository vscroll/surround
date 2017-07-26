#ifndef CAPTURE1WORKERBASE_H
#define CAPTURE1WORKERBASE_H

#include "common.h"
#include <queue>
#include "wrap_thread.h"

class Capture1WorkerBase
{
public:
    Capture1WorkerBase();
    virtual ~Capture1WorkerBase();

    int setCapCapacity(struct cap_sink_t* sink, struct cap_src_t* source);
    int setFocusSource(struct cap_src_t* focusSource);
    void getFocusSource(struct cap_src_t* focusSource);
    void clearFocusSource();
    virtual int openDevice(unsigned int channel) = 0;
    virtual void closeDevice() = 0;
    surround_image_t* popOneFrame();
    surround_image_t* popOneFrame4FocusSource();
    int getResolution(unsigned int* width, unsigned int* height);
    int getFPS(unsigned int* fps);
    unsigned int getFrameCount();
    void enableCapture();
    surround_image_t* captureOneFrame4FocusSource();

protected:
    void clearOverstock();
    bool isNeedConvert(struct cap_sink_t* sink, struct cap_src_t* source);

protected:
    struct cap_sink_t mSink;
    struct cap_src_t mSource;

    int mVideoFd;
    unsigned int mVideoChannel;

    std::queue<surround_image_t*> mSurroundImageQueue;
    pthread_mutex_t mMutexQueue;

    struct cap_src_t mFocusSource;
    pthread_mutex_t mMutexFocusSourceQueue;
    std::queue<surround_image_t*> mFocusSourceQueue;

    bool mEnableCapture;
    surround_image_t mCaptureFrame4FocusSource;

    clock_t mLastCallTime;
    unsigned int mRealFPS;
    clock_t mStartStatTime;
    clock_t mStatDuration;
    unsigned long mRealFrameCount;
};

#endif // CAPTURE1WORKERBASE_H
