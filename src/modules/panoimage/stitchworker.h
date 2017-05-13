#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include "common.h"
#include "thread.h"
#include <opencv/cv.h>
#include <queue>

class CLPano2D;
class StitchWorker : public Thread
{
public:
    StitchWorker();
    virtual ~StitchWorker();

    int init(unsigned int inWidth,
		    unsigned int inHeight,
		    unsigned int inPixfmt,        
		    unsigned int panoWidth,
		    unsigned int panoHeight,
		    unsigned int panoPixfmt,
		    char* algoCfgFilePath,
		    bool enableOpenCL);

    void queueImages(surround_images_t* surroundImages);
    surround_image_t* dequeuePanoImage();

public:
    virtual void run();

private:
    void clearOverstock();

    void stitching_init(const std::string config_path, cv::Mat& map, cv::Mat& mask, bool enableOpenCL);

    void stitching(const void* front, const void* rear, const void* left, const void* right,
               const cv::Mat& map, const cv::Mat& mask,
               void** outPano2D, int outPano2DWidth, int outPano2DHeight);

    void stitching_cl(const void* front, const void* rear, const void* left, const void* right,
                const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                void** outPano2D, int outPano2DWidth, int outPano2DHeight);
private:
    pthread_mutex_t mInputImagesMutex;
    std::queue<surround_images_t*> mInputImagesQueue;

    unsigned int mPanoWidth;
    unsigned int mPanoHeight;
    unsigned int mPanoPixfmt;
    unsigned int mPanoSize;
    pthread_mutex_t mOutputPanoImageMutex;
    std::queue<surround_image_t*> mOutputPanoImageQueue;

    cv::Mat mStitchMap;
    cv::Mat mStitchMask;

    cv::Mat mStitchMapAlignX;
    cv::Mat mStitchMapAlignY;
    cv::Mat mStitchMaskAlign;

    bool mEnableOpenCL;
    CLPano2D* mCLPano2D;

    double mLastCallTime;
};

#endif // STITCHWORKER_H
