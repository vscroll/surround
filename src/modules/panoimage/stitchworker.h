#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include "common.h"
#include "wrap_thread.h"
#include <opencv/cv.h>
#include <queue>
#include "imageshm.h"

class ICapture;
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

    void stitching_init(const std::string configPath,
        cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        bool enableOpenCL);

    void stitching(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);

    void stitching_cl(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);

private:
    class SourceSHMReadWorker : public WrapThread
    {
    public:
        SourceSHMReadWorker(ImageSHM* imageSHM);
        virtual ~SourceSHMReadWorker();

    public:
        virtual void run();

    private:
        ImageSHM* mImageSHM;

    protected:
        unsigned char mImageBuf[SHM_FRONT_SOURCE_SIZE];
        surround_image_t mImage;
        
        friend StitchWorker;
    };

    SourceSHMReadWorker* mSourceSHMReadWorker[VIDEO_CHANNEL_SIZE];

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

    cv::Mat* mLookupTab[VIDEO_CHANNEL_SIZE];
    cv::Mat mMask;
    cv::Mat mWeight;

    cv::Mat mStitchMapAlignX;
    cv::Mat mStitchMapAlignY;
    cv::Mat mStitchMaskAlign;

    bool mEnableOpenCL;
    CLPano2D* mCLPano2D;

    clock_t mLastCallTime;
};

#endif // STITCHWORKER_H
