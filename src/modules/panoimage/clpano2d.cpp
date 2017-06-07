#include "clpano2d.h"

#define IMX_OPENCL_ALLOC_ONCE 1
#define USE_MEM_VERSION_0 1

#define USE_MAP 0

CLPano2D::CLPano2D()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mMemSideImage[i] = NULL;
        mMemLookupTab[i] = NULL;
    }

    mMemPanoImage = NULL;
    mMapPanoImage = NULL;

    mBufferReady = FALSE;
    mOutputBufferReady = FALSE;
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
#if DEBUG_STITCH
    //clock_t start0 = clock();
#endif
#if USE_MEM_VERSION_0
    if (allocBuffer(sideImage, lookupTab, mask, weight, panoWidth, panoHeight, panoSize, panoImage) < 0)
    {
        freeBuffer();
        return -1;
    }

    if (writeBuffer(sideImage, lookupTab, mask, weight, panoWidth, panoHeight, panoSize, panoImage) < 0)
    {
        freeBuffer();
        return -1;
    }
#else
    if (allocOutputBuffer(sideImage, lookupTab, mask, weight, panoWidth, panoHeight, panoSize, panoImage) < 0)
    {
        freeOutputBuffer();
        return -1;
    }
    if (allocInputBuffer(sideImage, lookupTab, mask, weight, panoWidth, panoHeight, panoSize, panoImage) < 0)
    {
        freeInputBuffer();
        return -1;
    }
#endif

    size_t global[2];
    global[0] = panoWidth*2;
    global[1] = panoHeight;

#if DEBUG_STITCH
    //clock_t start1 = clock();
#endif
    ret = clEnqueueNDRangeKernel(mCQ, mKernel, 2, NULL, global, NULL, 0, NULL, NULL);
    //clFlush(mCQ);
    clFinish(mCQ);
    //clock_t start2 = clock();
#if USE_MAP
    if (mMapPanoImage != NULL)
    {
	    memcpy((void*)(panoImage), mMapPanoImage, panoSize*sizeof(uchar));
    }
#else
    ret |= clEnqueueReadBuffer(mCQ, mMemPanoImage, CL_TRUE, 0,
			panoSize*sizeof(uchar), (void*)(panoImage), 0, NULL, NULL);
    if  (ret != CL_SUCCESS)
    {
        printf ("\nError reading output buffer\n");
    }
#endif

#if DEBUG_STITCH
    //clock_t start3 = clock();
#endif

#if USE_MEM_VERSION_0
    // Allocate once
#if IMX_OPENCL_ALLOC_ONCE
#else
    freeBuffer();
#endif

#else
    freeInputBuffer();

    // Allocate once
#if IMX_OPENCL_ALLOC_ONCE
#else
    freeOutputBuffer();
#endif

#endif

#if DEBUG_STITCH
/*
    clock_t start4 = clock();
    printf ("\n stitch_cl_2d: write:%f cmd:%f read:%f del:%f total:%f\n",
		(start1-start0)/CLOCKS_PER_SEC,
		(start2-start1)/CLOCKS_PER_SEC,
		(start3-start2)/CLOCKS_PER_SEC,
		(start4-start3)/CLOCKS_PER_SEC,
		(start4-start0)/CLOCKS_PER_SEC	
		);
*/
#endif

    return 0;
}

int CLPano2D::allocBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage)
{
#if IMX_OPENCL_ALLOC_ONCE
    //allocation once
    if (mBufferReady)
    {
        return 0;
    }
#endif

    printf ("\nAllocation buffer start\n");

    cl_int ret;
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mMemSideImage[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY,
                                sideImage[i]->info.size*sizeof(uchar),
                                NULL, &ret);
        if (ret != CL_SUCCESS)
        {
            printf("\nFailed Allocation sideImage[%d] buffer ret=%d\n", i, ret);
            return -1;
        }
    }

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mMemLookupTab[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY,
                                lookupTab[i]->channels()*lookupTab[i]->cols*lookupTab[i]->rows*sizeof(uchar),
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
                                weight.channels()*weight.cols*weight.rows*sizeof(uchar),
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

    mBufferReady = TRUE;
    printf ("\nAllocation buffer ok\n");
    return 0;
}

void CLPano2D::freeBuffer()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        clReleaseMemObject(mMemSideImage[i]);
        clReleaseMemObject(mMemLookupTab[i]);
    }

    clReleaseMemObject(mMemMask);
    clReleaseMemObject(mMemWeight);
    clReleaseMemObject(mMemPanoImage);

    mBufferReady = FALSE;
}

int CLPano2D::writeBuffer(surround_image_t* sideImage[],
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
    //clock_t start0 = clock();
#endif

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
#if DEBUG_STITCH
        //clock_t start1 = clock();
#endif
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
#if DEBUG_STITCH
        //clock_t start2 = clock();
#endif
    }

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
#if DEBUG_STITCH
        //clock_t start3 = clock();
#endif
        ret = clEnqueueWriteBuffer(mCQ,
                               mMemLookupTab[i],
                               CL_TRUE, 0,
                               lookupTab[i]->channels()*lookupTab[i]->cols*lookupTab[i]->rows*sizeof(uchar),
                               (void*)(lookupTab[i]->data),
                               0, NULL, NULL);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed write buffer lookupTab[%d] ret=%d\n", i, ret);
            return -1;
        }
#if DEBUG_STITCH
        //clock_t start4 = clock();
#endif
    }

#if DEBUG_STITCH
    //clock_t start5 = clock();
#endif
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

#if DEBUG_STITCH
    //clock_t start6 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mMemWeight,
                               CL_TRUE, 0,
                               weight.channels()*weight.cols*weight.rows*sizeof(uchar),
                               (void*)(weight.data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer weight ret=%d %d %d %d\n", ret, weight.channels(), weight.cols, weight.rows);
        return -1;
    }

#if DEBUG_STITCH
/*    clock_t start7 = clock();
    printf ("\n = stitch_cl_write_pano2d_buffer: %f %f %f %f %f %f %f %f\n",
		(start1-start0)/CLOCKS_PER_SEC,
		(start2-start1)/CLOCKS_PER_SEC,
		(start3-start2)/CLOCKS_PER_SEC,
		(start4-start3)/CLOCKS_PER_SEC,
		(start5-start4)/CLOCKS_PER_SEC,
		(start6-start5)/CLOCKS_PER_SEC,
		(start7-start6)/CLOCKS_PER_SEC,
		(start7-start0)/CLOCKS_PER_SEC
		);
*/
#endif

    return 0;
}

int CLPano2D::allocOutputBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage)
{
    cl_int ret;
#if IMX_OPENCL_ALLOC_ONCE
    //allocation once
    if (mOutputBufferReady)
    {
        return 0;
    }
#endif

    printf ("\nAllocation buffer start\n");

    int size = panoSize*sizeof(uchar);
    mMemPanoImage = clCreateBuffer(mContext,
                        CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
                        size, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer panoimage %d\n", ret);
        return -1;
    }

#if USE_MAP
    mMapPanoImage = clEnqueueMapBuffer(mCQ, mMemPanoImage, CL_TRUE, CL_MAP_READ, 0,
		                size, 0, NULL, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Map panoimage buffer\n");
        return -1;
    }
#endif

    mOutputBufferReady = TRUE;
    printf ("\nAllocation buffer ok\n");

    return 0;
}

void CLPano2D::freeOutputBuffer()
{
    clEnqueueUnmapMemObject(mCQ, mMemPanoImage, mMapPanoImage, 0, NULL, NULL);
    clReleaseMemObject(mMemPanoImage);
    mOutputBufferReady = FALSE;
}

int CLPano2D::allocInputBuffer(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage)
{
    cl_int ret;

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
#if DEBUG_STITCH
        //clock_t start0 = clock();
#endif
        mMemSideImage[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                sideImage[i]->info.size*sizeof(uchar),
                                (void*)(sideImage[i]->data),
                                &ret);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed Allocation buffer sideimage[%d] ret=%d\n", i, ret);
            return -1;
        }

#if DEBUG_STITCH
        //clock_t start1 = clock();
#endif
    }

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
#if DEBUG_STITCH
        //clock_t start2 = clock();
#endif
        mMemLookupTab[i] = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                                lookupTab[i]->channels()*lookupTab[i]->cols*lookupTab[i]->rows*sizeof(uchar),
                                (void*)(lookupTab[i]->data),
                                &ret);
        if (ret != CL_SUCCESS)
        {
            printf ("\nFailed Allocation buffer sideimage[%d] ret=%d\n", i, ret);
            return -1;
        }

#if DEBUG_STITCH
        //clock_t start3 = clock();
#endif
    }


#if DEBUG_STITCH
    //clock_t start4 = clock();
#endif
    mMemMask = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                            mask.channels()*mask.cols*mask.rows*sizeof(uchar),
                            (void*)(mask.data),
                            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer mask %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start5 = clock();
#endif
    mMemWeight = clCreateBuffer(mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                            weight.channels()*weight.cols*weight.rows*sizeof(uchar),
                            (void*)(weight.data),
                            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer weight %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start6 = clock();
#endif

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

#if DEBUG_STITCH
/*
    printf ("\n stitch_cl_write_pano2d_buffer2: front:%f rear:%f left:%f right:%f mask:%f mapx:%f mapy:%f total:%f\n",
		(start1-start0)/CLOCKS_PER_SEC,
		(start2-start1)/CLOCKS_PER_SEC,
		(start3-start2)/CLOCKS_PER_SEC,
		(start4-start3)/CLOCKS_PER_SEC,
		(start5-start4)/CLOCKS_PER_SEC,
		(start6-start5)/CLOCKS_PER_SEC
		);
*/
#endif
    return 0;
}

void CLPano2D::freeInputBuffer()
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        clReleaseMemObject(mMemSideImage[i]);
        clReleaseMemObject(mMemLookupTab[i]);
    }

    clReleaseMemObject(mMemMask);
    clReleaseMemObject(mMemWeight);
}

