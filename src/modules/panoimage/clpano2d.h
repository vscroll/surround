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
    int allocAllBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize);
    int writeAllBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight);
    void freeAllBuffer();

    int allocAndWritePanoBuffer(unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize);
    void freePanoBuffer();

    int allocAndWriteLookupTabBuffer(cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight);
    void freeLookupTabBuffer();

    int allocAndWriteSideBuffer(surround_image_t* sideImage[]);
    void freeSideBuffer();


private:
    cl_kernel mKernel;

    cl_mem mMemSideImage[VIDEO_CHANNEL_SIZE];
    cl_mem mMemLookupTab[VIDEO_CHANNEL_SIZE];
    cl_mem mMemMask;
    cl_mem mMemWeight;

    cl_mem mMemPanoImage;
    void* mMapPanoImage;

    bool mAllBufReady;
    bool mLookupTabBufReady;
    bool mPanoBufReady;
};

#endif // CLPANO2D_H
