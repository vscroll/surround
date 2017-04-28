#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include "common.h"
#include "thread.h"
#include <opencv/cv.h>
#include <queue>

class ICapture;
class StitchWorker : public Thread
{
public:
    StitchWorker();
    virtual ~StitchWorker();

    void init(ICapture *capture,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		char* configFilePath,
		bool enableOpenCL);

    surround_image_t* dequeuePano2DImage();
    surround_image_t* dequeueSideImage(unsigned int channelIndex);

public:
    virtual void run();

private:
    unsigned int mPano2DWidth;
    unsigned int mPano2DHeight;

    std::queue<surround_image_t*> mOutputPano2DImageQueue;
    std::queue<surround_image_t*> mOutputSideImageQueue;

    pthread_mutex_t mOutputPano2DImageMutex;
    pthread_mutex_t mOutputSideImageMutex;

    unsigned int mCurChannelIndex;

    ICapture *mCapture;

    cv::Mat mStitchMap;
    cv::Mat mStitchMapX;
    cv::Mat mStitchMapY;
    cv::Mat mMask;

    double mLastTimestamp;

    bool mEnableOpenCL;
};

#endif // STITCHWORKER_H
