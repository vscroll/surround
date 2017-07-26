#include "clpanorender.h"

CLPanoRender::CLPanoRender()
{

}

CLPanoRender::~CLPanoRender()
{

}

int CLPanoRender::stitch(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage)
{
    cl_int ret;

#if DEBUG_STITCH
    clock_t start0 = clock();
#endif
    if (allocAndWriteSideBuffer(sideImage) < 0)
    {
        freeSideBuffer();
        return -1;
    }

#if DEBUG_STITCH
    clock_t start1 = clock();
#endif
    if (!mLookupTabBufReady)
    {
        if (allocAndWriteLookupTabBuffer(lookupTab, mask, weight) < 0)
        {
            freeLookupTabBuffer();
            return -1;
        }

        mLookupTabBufReady = true;
    }

#if DEBUG_STITCH
    clock_t start2 = clock();
#endif
    if (!mPanoBufReady)
    {
        if (allocAndWritePanoBuffer(panoWidth, panoHeight, panoSize) < 0)
        {
            freePanoBuffer();
            return -1;
        }

        mPanoBufReady = true;
    }

    size_t global[2];
    global[0] = panoWidth*2;
    global[1] = panoHeight;

#if DEBUG_STITCH
    clock_t start3 = clock();
#endif

    ret = clEnqueueNDRangeKernel(mCQ, mKernel, 2, NULL, global, NULL, 0, NULL, NULL);
    //clFlush(mCQ);
    clFinish(mCQ);
#if DEBUG_STITCH
    clock_t start4 = clock();
#endif

    ret |= clEnqueueReadBuffer(mCQ, mMemPanoImage, CL_TRUE, 0,
			panoSize*sizeof(uchar), (void*)(panoImage), 0, NULL, NULL);

    if  (ret != CL_SUCCESS)
    {
        printf ("\nError reading output buffer\n");
    }

#if DEBUG_STITCH
    clock_t start5 = clock();
#endif

    freeSideBuffer();

#if DEBUG_STITCH
    clock_t start6 = clock();
    printf ("\nCLPanoRender: stitch_cl_2d side:%f lut:%f pano:%f exe:%f read:%f release:%f total:%f\n",
		(double)(start1-start0)/CLOCKS_PER_SEC,
		(double)(start2-start1)/CLOCKS_PER_SEC,
		(double)(start3-start2)/CLOCKS_PER_SEC,
		(double)(start4-start3)/CLOCKS_PER_SEC,
		(double)(start5-start4)/CLOCKS_PER_SEC,
		(double)(start6-start5)/CLOCKS_PER_SEC,
		(double)(start6-start0)/CLOCKS_PER_SEC);
#endif

    return 0;
}
