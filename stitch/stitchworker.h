#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include "common.h"
#include "thread.h"
#include <opencv/cv.h>
#include <queue>

class ICapture;
class CLPano2D;
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
    void stitching_init(const std::string config_path, cv::Mat& map, cv::Mat& mask, bool enableOpenCL);

    void stitching(const void* front, const void* rear, const void* left, const void* right,
               const cv::Mat& map, const cv::Mat& mask,
               void** outPano2D, int outPano2DWidth, int outPano2DHeight,
               void** outSide, int outSideWidth, int outSideHeight, int outSideChannel);

    void stitching_cl(const void* front, const void* rear, const void* left, const void* right,
                const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                void** outPano2D, int outPano2DWidth, int outPano2DHeight,
                void** outSide, int outSideWidth, int outSideHeight, int outSideChannel);
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
    cv::Mat mStitchMask;

    cv::Mat mStitchMapAlignX;
    cv::Mat mStitchMapAlignY;
    cv::Mat mStitchMaskAlign;

    double mLastTimestamp;

    bool mEnableOpenCL;
    CLPano2D* mCLPano2D;
};

#endif // STITCHWORKER_H
