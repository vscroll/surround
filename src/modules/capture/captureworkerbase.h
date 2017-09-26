#ifndef CAPTUREWORKERBASE_H
#define CAPTUREWORKERBASE_H

#include <queue>
#include "common.h"
#include "wrap_thread.h"

class CaptureWorkerBase
{
public:
    CaptureWorkerBase();
    virtual ~CaptureWorkerBase();

    virtual int setCapCapacity(struct cap_sink_t sink[], struct cap_src_t source[], unsigned int channelNum);
    virtual int setFocusSource(unsigned int focusChannelIndex, struct cap_src_t* focusSource);
    virtual unsigned int getFocusChannelIndex();
    virtual int openDevice(unsigned int channel[], unsigned int channelNum) = 0;
    virtual void closeDevice() = 0;
    virtual surround_images_t* popOneFrame();
    virtual surround_image_t* popOneFrame4FocusSource();
    virtual unsigned int getVideoChannelNum() { return mVideoChannelNum; }
    virtual int getResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height);
    virtual int getFPS(unsigned int* fps);
    virtual void enableCapture();
    virtual surround_image_t* captureOneFrame4FocusSource();
    virtual surround_image_t* popOneFrame(unsigned int channelIndex);

protected:
    void clearOverstock();
    bool isNeedConvert(struct cap_sink_t* sink, struct cap_src_t* source);

protected:
    struct cap_sink_t mSink[VIDEO_CHANNEL_SIZE];
    struct cap_src_t mSource[VIDEO_CHANNEL_SIZE];

    int mVideoFd[VIDEO_CHANNEL_SIZE];
    unsigned int mVideoChannelNum;
    unsigned int mChannel[VIDEO_CHANNEL_SIZE];

    pthread_mutex_t mMutexQueue[VIDEO_CHANNEL_SIZE];
    std::queue<surround_image_t*> mSurroundImageQueue[VIDEO_CHANNEL_SIZE];

    unsigned int mFocusChannelIndex;
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

#endif // CAPTUREWORKERBASE_H
