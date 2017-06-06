#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include "common.h"
#include "wrap_thread.h"
#include <opencv/cv.h>
#include <queue>

class ICapture;
class ImageSHM;
class CLPano2D;
class StitchWorker : public WrapThread
{
public:
    StitchWorker();
    virtual ~StitchWorker();

    int init(
            ICapture* capture,
            unsigned int inWidth,
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

    void stitching_init(
        const std::string configPath,
        cv::Mat& LutFront,
    	cv::Mat& LutRear,
    	cv::Mat& LutLeft,
    	cv::Mat& LutRight,
    	cv::Mat& mask,
    	cv::Mat& weight,
        bool enableOpenCL);

    void stitching(
        const unsigned char* front,
        const unsigned char* rear,
    	const unsigned char* left,
    	const unsigned char* right,
    	const cv::Mat& LutFront,
    	const cv::Mat& LutRear,
    	const cv::Mat& LutLeft,
    	const cv::Mat& LutRight,
    	const cv::Mat& mask,
    	const cv::Mat& weight,
        unsigned char* outPano,
        unsigned int outPanoWidth,
        unsigned int outPanoHeight,
        unsigned int outPanoSize);

    void stitching_cl(const void* front, const void* rear, const void* left, const void* right,
                const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                void** outPano2D, int outPano2DWidth, int outPano2DHeight);
private:
    ICapture* mCapture;
    ImageSHM* mImageSHM[VIDEO_CHANNEL_SIZE];

    pthread_mutex_t mInputImagesMutex;
    std::queue<surround_images_t*> mInputImagesQueue;

    unsigned int mPanoWidth;
    unsigned int mPanoHeight;
    unsigned int mPanoPixfmt;
    unsigned int mPanoSize;
    pthread_mutex_t mOutputPanoImageMutex;
    std::queue<surround_image_t*> mOutputPanoImageQueue;

    cv::Mat mLutFront;
    cv::Mat mLutRear;
    cv::Mat mLutLeft;
    cv::Mat mLutRight;
    cv::Mat mMask;
    cv::Mat mWeight;

    cv::Mat mStitchMap;
    cv::Mat mStitchMask;

    cv::Mat mStitchMapAlignX;
    cv::Mat mStitchMapAlignY;
    cv::Mat mStitchMaskAlign;

    bool mEnableOpenCL;
    CLPano2D* mCLPano2D;

    clock_t mLastCallTime;
};

#endif // STITCHWORKER_H
