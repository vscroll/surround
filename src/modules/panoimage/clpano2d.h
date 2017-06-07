#ifndef CLPANO2D_H
#define CLPANO2D_H

#include "clbase.h"
#include "common.h"
#include <CL/cl.h>
#include <opencv/cv.h>

class CLPano2D : public CLBase
{
public:
    CLPano2D();
    virtual ~CLPano2D();

public:
    int init(char* clFileName, char* clKernelName);
    int uninit();

    int stitch(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);
private:
    int allocBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);
    int writeBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);
    void freeBuffer();

    int allocOutputBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);
    void freeOutputBuffer();
    int allocInputBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);
    void freeInputBuffer();
private:
    cl_kernel mKernel;

    cl_mem mMemSideImage[VIDEO_CHANNEL_SIZE];
    cl_mem mMemLookupTab[VIDEO_CHANNEL_SIZE];
    cl_mem mMemMask;
    cl_mem mMemWeight;

    cl_mem mMemPanoImage;
    void* mMapPanoImage;

    int mBufferReady ;
    int mOutputBufferReady;
};

#endif // CLPANO2D_H
