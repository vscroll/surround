#include "clpano2d.h"

#define USE_MEM_VERSION_0 0

#if (USE_MEM_VERSION_0 == 0)
#define USE_MAP 0
#endif

CLPano2D::CLPano2D()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mMemSideImage[i] = NULL;
        mMemLookupTab[i] = NULL;
    }

    mMemPanoImage = NULL;
    mMapPanoImage = NULL;

    mAllBufReady = false;
    mLookupTabBufReady = false;
    mPanoBufReady = false;
}

CLPano2D::~CLPano2D()
{

}

int CLPano2D::init(char* clFileName, char* clKernelName)
{
    int ret;
    ret = initEnv();
    if (ret < 0)
    {
        return ret;
    }

    return loadKernel(clFileName, clKernelName, &mKernel);
}

int CLPano2D::uninit()
{
    releaseKernel(mKernel);
    uninitEnv();

    return 0;
}

int CLPano2D::stitch(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage)
{
    cl_int ret;
#if USE_MEM_VERSION_0
    if (!mAllBufReady)
    {
        if (allocAllBuffer(sideImage, lookupTab, mask, weight, panoWidth, panoHeight, panoSize) < 0)
        {
            freeAllBuffer();
            return -1;
        }

        mAllBufReady = true;
    }

    if (writeAllBuffer(sideImage, lookupTab, mask, weight) < 0)
    {
        return -1;
    }
#else

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
#endif

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

#if USE_MAP
    if (mMapPanoImage != NULL)
    {
	    memcpy((void*)(panoImage), mMapPanoImage, panoSize*sizeof(uchar));
    }
#else
    ret |= clEnqueueReadBuffer(mCQ, mMemPanoImage, CL_TRUE, 0,
			panoSize*sizeof(uchar), (void*)(panoImage), 0, NULL, NULL);
#endif
    if  (ret != CL_SUCCESS)
    {
        printf ("\nError reading output buffer\n");
    }

#if DEBUG_STITCH
    clock_t start5 = clock();
#endif

#if USE_MEM_VERSION_0

#else
    freeSideBuffer();

#endif

#if DEBUG_STITCH
    clock_t start6 = clock();
    printf ("\nstitch_cl_2d: side:%f lut:%f pano:%f exe:%f read:%f release:%f total:%f\n",
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

int CLPano2D::allocAllBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize)
{
    printf ("\nAllocation buffer all start\n");

    cl_int ret;
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        if (sideImage[i]->info.width == 0
            || sideImage[i]->info.height == 0
            || sideImage[i]->info.size == 0)
        {
            printf ("\nFailed Allocation buffer sideimage[%d] width=%d height=%d size=%d ret=%d\n",
                i, sideImage[i]->info.width, sideImage[i]->info.height, sideImage[i]->info.size, ret);
            return -1;
        }

        mMemSideImage[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY,
                                sideImage[i]->info.size*sizeof(uchar),
                                NULL, &ret);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed Allocation buffer sideimage[%d] width=%d height=%d size=%d ret=%d\n",
                i, sideImage[i]->info.width, sideImage[i]->info.height, sideImage[i]->info.size, ret);
            return -1;
        }
    }

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mMemLookupTab[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY,
                                lookupTab[i]->channels()*lookupTab[i]->cols*lookupTab[i]->rows*sizeof(float),
                                NULL, &ret);
        if (ret != CL_SUCCESS)
        {
            printf("\nFailed Allocation lookupTable[%d] buffer ret=%d\n", i, ret);
            return -1;
        }
    }

    mMemMask = clCreateBuffer(mContext, CL_MEM_READ_ONLY,
                                mask.channels()*mask.cols*mask.rows*sizeof(uchar),
                                NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation mask buffer ret=%d %d %d %d\n", ret, mask.channels(), mask.cols, mask.rows);
         return -1;
    }

    mMemWeight = clCreateBuffer(mContext, CL_MEM_READ_ONLY,
                                weight.channels()*weight.cols*weight.rows*sizeof(float),
                                NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation weight buffer ret=%d %d %d %d\n", ret, weight.channels(), weight.cols, weight.rows);
         return -1;
    }

    mMemPanoImage = clCreateBuffer(mContext, CL_MEM_WRITE_ONLY,
                                panoSize*sizeof(uchar),
                                NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation g_image_pano2d output buffer ret=%d\n", ret);
         return -1;
    }

    int argc = 0;
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        ret = clSetKernelArg(mKernel, argc++, sizeof(cl_mem), &mMemSideImage[i]);
    }

    ret |= clSetKernelArg(mKernel, argc++, sizeof(int), &(sideImage[0]->info.width));
    ret |= clSetKernelArg(mKernel, argc++, sizeof(int), &(sideImage[0]->info.height));

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        ret |= clSetKernelArg (mKernel, argc++, sizeof(cl_mem), &mMemLookupTab[i]);
    }

    ret |= clSetKernelArg(mKernel, argc++, sizeof(cl_mem), &mMemMask);
    ret |= clSetKernelArg(mKernel, argc++, sizeof(cl_mem), &mMemWeight);
    ret |= clSetKernelArg(mKernel, argc++, sizeof(int), &(panoWidth));
    ret |= clSetKernelArg(mKernel, argc++, sizeof(int), &(panoHeight));
    ret |= clSetKernelArg(mKernel, argc++, sizeof(cl_mem), &mMemPanoImage);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed set kernel arg\n");
         return -1;
    }

    printf ("\nAllocation buffer all ok\n");
    return 0;
}

int CLPano2D::writeAllBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight)
{
    cl_int ret;
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        ret = clEnqueueWriteBuffer(mCQ,
                               mMemSideImage[i],
                               CL_TRUE, 0,
                               sideImage[i]->info.size*sizeof(uchar),
                               (void*)(sideImage[i]->data),
                               0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed write buffer sideImage[%d] ret=%d\n", i, ret);
            return -1;
        }
    }

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        ret = clEnqueueWriteBuffer(mCQ,
                                mMemLookupTab[i],
                                CL_TRUE, 0,
                                lookupTab[i]->channels()*lookupTab[i]->cols*lookupTab[i]->rows*sizeof(float),
                                (void*)(lookupTab[i]->data),
                                0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed write buffer lookupTab[%d] ret=%d\n", i, ret);
            return -1;
        }
    }

    ret = clEnqueueWriteBuffer(mCQ,
                                mMemMask,
                                CL_TRUE, 0,
                                mask.channels()*mask.cols*mask.rows*sizeof(uchar),
                                (void*)(mask.data),
                                0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer mask ret=%d %d %d %d\n", ret, mask.channels(), mask.cols, mask.rows);
        return -1;
    }

    ret = clEnqueueWriteBuffer(mCQ,
                                mMemWeight,
                                CL_TRUE, 0,
                                weight.channels()*weight.cols*weight.rows*sizeof(float),
                                (void*)(weight.data),
                                0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer weight ret=%d %d %d %d\n", ret, weight.channels(), weight.cols, weight.rows);
        return -1;
    }

    return 0;
}

void CLPano2D::freeAllBuffer()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        clReleaseMemObject(mMemSideImage[i]);
        clReleaseMemObject(mMemLookupTab[i]);
    }

    clReleaseMemObject(mMemMask);
    clReleaseMemObject(mMemWeight);
    clReleaseMemObject(mMemPanoImage);
}

int CLPano2D::allocAndWriteSideBuffer(surround_image_t* sideImage[])
{
    cl_int ret;

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        if (sideImage[i]->info.width == 0
            || sideImage[i]->info.height == 0
            || sideImage[i]->info.size == 0)
        {
            printf ("\nFailed Allocation buffer sideimage[%d] width=%d height=%d size=%d ret=%d\n",
                i, sideImage[i]->info.width, sideImage[i]->info.height, sideImage[i]->info.size, ret);
            return -1;
        }

        mMemSideImage[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                sideImage[i]->info.size*sizeof(uchar),
                                (void*)(sideImage[i]->data),
                                &ret);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed Allocation buffer sideimage[%d] width=%d height=%d size=%d ret=%d\n",
                i, sideImage[i]->info.width, sideImage[i]->info.height, sideImage[i]->info.size, ret);
            return -1;
        }
    }

    ret = clSetKernelArg(mKernel, 0, sizeof(cl_mem), &mMemSideImage[VIDEO_CHANNEL_FRONT]);
    ret |= clSetKernelArg(mKernel, 1, sizeof(cl_mem), &mMemSideImage[VIDEO_CHANNEL_REAR]);
    ret |= clSetKernelArg(mKernel, 2, sizeof(cl_mem), &mMemSideImage[VIDEO_CHANNEL_LEFT]);
    ret |= clSetKernelArg(mKernel, 3, sizeof(cl_mem), &mMemSideImage[VIDEO_CHANNEL_RIGHT]);

    ret |= clSetKernelArg(mKernel, 4, sizeof(int), &(sideImage[VIDEO_CHANNEL_FRONT]->info.width));
    ret |= clSetKernelArg(mKernel, 5, sizeof(int), &(sideImage[VIDEO_CHANNEL_FRONT]->info.height));
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed set kernel side arg\n");
         return -1;
    }

    return 0;
}

void CLPano2D::freeSideBuffer()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        clReleaseMemObject(mMemSideImage[i]);
    }
}

int CLPano2D::allocAndWriteLookupTabBuffer(cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight)
{
    cl_int ret;

    printf ("\nAllocation buffer lookuptab start\n");

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mMemLookupTab[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                lookupTab[i]->channels()*lookupTab[i]->cols*lookupTab[i]->rows*sizeof(float),
                                (void*)(lookupTab[i]->data),
                                &ret);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed Allocation buffer lookuptab[%d] ret=%d\n", i, ret);
            return -1;
        }
    }

    mMemMask = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                        mask.channels()*mask.cols*mask.rows*sizeof(uchar),
                        (void*)(mask.data),
                        &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer mask %d\n", ret);
        return -1;
    }

    mMemWeight = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            weight.channels()*weight.cols*weight.rows*sizeof(float),
            (void*)(weight.data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer weight %d\n", ret);
        return -1;
    }

    ret = clSetKernelArg (mKernel, 6, sizeof(cl_mem), &mMemLookupTab[VIDEO_CHANNEL_FRONT]);
    ret |= clSetKernelArg (mKernel, 7, sizeof(cl_mem), &mMemLookupTab[VIDEO_CHANNEL_REAR]);
    ret |= clSetKernelArg (mKernel, 8, sizeof(cl_mem), &mMemLookupTab[VIDEO_CHANNEL_LEFT]);
    ret |= clSetKernelArg (mKernel, 9, sizeof(cl_mem), &mMemLookupTab[VIDEO_CHANNEL_RIGHT]);
    ret |= clSetKernelArg(mKernel, 10, sizeof(cl_mem), &mMemMask);
    ret |= clSetKernelArg(mKernel, 11, sizeof(cl_mem), &mMemWeight);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed set kernel lookuptab arg\n");
         return -1;
    }

    printf ("\nAllocation buffer lookuptab ok\n");
    return 0;
}

void CLPano2D::freeLookupTabBuffer()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        clReleaseMemObject(mMemLookupTab[i]);
    }

    clReleaseMemObject(mMemMask);
    clReleaseMemObject(mMemWeight);
}

int CLPano2D::allocAndWritePanoBuffer(unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize)
{
    cl_int ret;

    printf ("\nAllocation buffer pano start\n");

    int size = panoSize*sizeof(uchar);
    mMemPanoImage = clCreateBuffer(mContext,
                        CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                        size, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer pano %d\n", ret);
        return -1;
    }

#if USE_MAP
    mMapPanoImage = clEnqueueMapBuffer(mCQ, mMemPanoImage, CL_TRUE, CL_MAP_READ, 0,
		                size, 0, NULL, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Map pano buffer\n");
        return -1;
    }
#endif

    ret = clSetKernelArg(mKernel, 12, sizeof(int), &(panoWidth));
    ret |= clSetKernelArg(mKernel, 13, sizeof(int), &(panoHeight));
    ret |= clSetKernelArg(mKernel, 14, sizeof(cl_mem), &mMemPanoImage);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed set kernel arg\n");
         return -1;
    }

    printf ("\nAllocation buffer pano ok\n");

    return 0;
}

void CLPano2D::freePanoBuffer()
{
    clEnqueueUnmapMemObject(mCQ, mMemPanoImage, mMapPanoImage, 0, NULL, NULL);
    clReleaseMemObject(mMemPanoImage);
}


