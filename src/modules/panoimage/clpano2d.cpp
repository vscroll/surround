#include "clpano2d.h"

#define IMX_OPENCL_ALLOC_ONCE 1
#define USE_MEM_VERSION_0 0

#define USE_MAP 1

CLPano2D::CLPano2D()
{
    mImageFront = NULL;
    mImageRear = NULL;
    mImageLeft = NULL;
    mImageRight = NULL;
    mImageMask = NULL;
    mImageMapX = NULL;
    mImageMapY = NULL;

    mImagePano2d = NULL;
    mImagePano2dMap = NULL;

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
}

int CLPano2D::stitch_cl_2d(const std::vector<cv::Mat>& side_imgs,
                 const cv::Mat& map_x, const cv::Mat& map_y,
                 const cv::Mat& mask,
                 cv::Mat& image_pano2d
                 )
{
    cl_int ret;
#if DEBUG_STITCH
    //clock_t start0 = clock();
#endif
#if USE_MEM_VERSION_0
    if (stitch_cl_new_pano2d_buffer(side_imgs, map_x, map_y, mask, image_pano2d) < 0)
    {
        stitch_cl_delete_pano2d_buffer();
        return -1;
    }

    if (stitch_cl_write_pano2d_buffer(side_imgs, map_x, map_y, mask) < 0)
    {
        stitch_cl_delete_pano2d_buffer();
        return -1;
    }
#else
    if (stitch_cl_new_output_buffer(side_imgs, map_x, map_y, mask, image_pano2d) < 0)
    {
        stitch_cl_delete_pano2d_buffer();
        return -1;
    }
    if (stitch_cl_new_input_buffer(side_imgs, map_x, map_y, mask, image_pano2d) < 0)
    {
        stitch_cl_delete_pano2d_buffer();
        return -1;
    }
#endif

    size_t global[2];
    global[0] = side_imgs[0].cols;
    global[1] = side_imgs[0].rows;

#if DEBUG_STITCH
    //clock_t start1 = clock();
#endif
    ret = clEnqueueNDRangeKernel (mCQ, mKernel, 2, NULL, global, NULL, 0, NULL, NULL);
    //clFlush(mCQ);
    clFinish(mCQ);
    //clock_t start2 = clock();
#if USE_MAP
    if (mImagePano2dMap != NULL)
    {
	    memcpy((void*)(image_pano2d.data), mImagePano2dMap, image_pano2d.channels()*image_pano2d.cols*image_pano2d.rows*sizeof(uchar));
    }
#else
    ret |= clEnqueueReadBuffer(gCQ, mImagePano2d, CL_TRUE, 0,
			image_pano2d.channels()*image_pano2d.cols*image_pano2d.rows*sizeof(uchar),
                        (void*)(image_pano2d.data), 0, NULL, NULL);
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
    stitch_cl_delete_pano2d_buffer();
#endif

#else
    stitch_cl_delete_input_buffer();

    // Allocate once
#if IMX_OPENCL_ALLOC_ONCE
#else
    stitch_cl_delete_output_buffer();
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

int CLPano2D::stitch_cl_new_pano2d_buffer(const std::vector<cv::Mat>& side_imgs,
                                const cv::Mat& map_x, const cv::Mat& map_y,
                                const cv::Mat& mask,
                                cv::Mat& image_pano2d)
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
    mImageFront = clCreateBuffer (mContext, CL_MEM_READ_ONLY,
                                    side_imgs[0].channels()*side_imgs[0].cols*side_imgs[0].rows*sizeof(uchar),
                                    NULL,
                                   &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation  image_front buffer ret=%d %d %d %d\n", ret, side_imgs[0].channels(), side_imgs[0].cols, side_imgs[0].rows);
        return -1;
    }

    mImageRear = clCreateBuffer (mContext, CL_MEM_READ_ONLY,
                                   side_imgs[1].channels()*side_imgs[1].cols*side_imgs[1].rows*sizeof(uchar),
                                    NULL,
                                  &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation  image_rear buffer ret=%d %d %d %d\n",  ret, side_imgs[1].channels(), side_imgs[1].cols, side_imgs[1].rows);
         return -1;
    }

    mImageLeft = clCreateBuffer (mContext, CL_MEM_READ_ONLY,
                                   side_imgs[2].channels()*side_imgs[2].cols*side_imgs[2].rows*sizeof(uchar),
                                    NULL,
                                  &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation  image_left buffer ret=%d %d %d %d\n", ret, side_imgs[2].channels(), side_imgs[2].cols, side_imgs[2].rows);
         return -1;
    }

    mImageRight = clCreateBuffer (mContext, CL_MEM_READ_ONLY,
                                    side_imgs[3].channels()*side_imgs[3].cols*side_imgs[3].rows*sizeof(uchar),
                                    NULL,
                                   &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation  image_right buffer ret=%d %d %d %d\n", ret, side_imgs[3].channels(), side_imgs[3].cols, side_imgs[3].rows);
         return -1;
    }

    mImageMask  = clCreateBuffer (mContext, CL_MEM_READ_ONLY,
                                    mask.channels()*mask.cols*mask.rows*sizeof(uchar),
                                    NULL,
                                    &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation  image_right buffer ret=%d %d %d %d\n", ret, mask.channels(), mask.cols, mask.rows);
         return -1;
    }

    mImageMapX = clCreateBuffer (mContext, CL_MEM_READ_ONLY,
                                    map_x.channels()*map_x.cols*map_x.rows*sizeof(uchar),
                                    NULL,
                                    &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation  image_right buffer ret=%d %d %d %d\n", ret, map_x.channels(), map_x.cols, map_x.rows);
         return -1;
    }

    mImageMapY = clCreateBuffer (mContext, CL_MEM_READ_ONLY,
                                    map_y.channels()*map_y.cols*map_y.rows*sizeof(uchar),
                                    NULL,
                                    &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation  image_right buffer ret=%d %d %d %d\n", ret, map_y.channels(), map_y.cols, map_y.rows);
         return -1;
    }

    mImagePano2d = clCreateBuffer (mContext, CL_MEM_WRITE_ONLY,
                                     image_pano2d.channels()*image_pano2d.cols* image_pano2d.rows*sizeof(uchar),
                                     NULL,
                                     &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Allocation g_image_pano2d output buffer ret=%d %d %d %d\n", ret, image_pano2d.channels(), image_pano2d.cols, image_pano2d.rows);
         return -1;
    }

    ret = clSetKernelArg (mKernel, 0, sizeof(cl_mem), &mImageFront);
    ret |= clSetKernelArg (mKernel, 1, sizeof(cl_mem), &mImageRear);
    ret |= clSetKernelArg (mKernel, 2, sizeof(cl_mem), &mImageLeft);
    ret |= clSetKernelArg (mKernel, 3, sizeof(cl_mem), &mImageRight);
    ret |= clSetKernelArg (mKernel, 4, sizeof(int), &(side_imgs[0].cols));
    ret |= clSetKernelArg (mKernel, 5, sizeof(int), &(side_imgs[0].rows));
    ret |= clSetKernelArg (mKernel, 6, sizeof(cl_mem), &mImageMask);
    ret |= clSetKernelArg (mKernel, 7, sizeof(cl_mem), &mImageMapX);
    ret |= clSetKernelArg (mKernel, 8, sizeof(cl_mem), &mImageMapY);
    ret |= clSetKernelArg (mKernel, 9, sizeof(cl_mem), &mImagePano2d);
    ret |= clSetKernelArg (mKernel, 10, sizeof(int), &(image_pano2d.cols));
    ret |= clSetKernelArg (mKernel, 11, sizeof(int), &(image_pano2d.rows));
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed set kernel arg\n");
         return -1;
    }

    mBufferReady = TRUE;
    printf ("\nAllocation buffer ok\n");
    return 0;
}

void CLPano2D::stitch_cl_delete_pano2d_buffer()
{
    clReleaseMemObject (mImageFront);
    clReleaseMemObject (mImageRear);
    clReleaseMemObject (mImageLeft);
    clReleaseMemObject (mImageRight);
    clReleaseMemObject (mImageMask);
    clReleaseMemObject (mImageMapX);
    clReleaseMemObject (mImageMapY);
    clReleaseMemObject (mImagePano2d);

    mBufferReady = FALSE;
}

int CLPano2D::stitch_cl_write_pano2d_buffer(const std::vector<cv::Mat>& side_imgs,
                                       const cv::Mat& map_x, const cv::Mat& map_y,
                                       const cv::Mat& mask)
{
    cl_int ret;
#if DEBUG_STITCH
    //clock_t start0 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mImageFront,
                               CL_TRUE, 0,
                               side_imgs[0].channels()*side_imgs[0].cols*side_imgs[0].rows*sizeof(uchar),
                               (void*)(side_imgs[0].data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer g_image_front ret=%d %d %d %d\n", ret, side_imgs[0].channels(), side_imgs[0].cols, side_imgs[0].rows);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start1 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mImageRear,
                               CL_TRUE, 0,
                               side_imgs[1].channels()*side_imgs[1].cols*side_imgs[1].rows*sizeof(uchar),
                               (void*)(side_imgs[1].data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer g_image_rear ret=%d %d %d %d\n",  ret, side_imgs[1].channels(), side_imgs[1].cols, side_imgs[1].rows);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start2 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mImageLeft,
                               CL_TRUE, 0,
                               side_imgs[2].channels()*side_imgs[2].cols*side_imgs[2].rows*sizeof(uchar),
                               (void*)(side_imgs[2].data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer g_image_left ret=%d %d %d %d\n", ret, side_imgs[2].channels(), side_imgs[2].cols, side_imgs[2].rows);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start3 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mImageRight,
                               CL_TRUE, 0,
                               side_imgs[3].channels()*side_imgs[3].cols*side_imgs[3].rows*sizeof(uchar),
                               (void*)(side_imgs[3].data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer g_image_right ret=%d %d %d %d\n", ret, side_imgs[3].channels(), side_imgs[3].cols, side_imgs[3].rows);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start4 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mImageMapX,
                               CL_TRUE, 0,
                               map_x.channels()*map_x.cols*map_x.rows*sizeof(uchar),
                               (void*)(map_x.data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer g_image_map_x ret=%d %d %d %d\n", ret, map_x.channels(), map_x.cols, map_x.rows);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start5 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mImageMapY,
                               CL_TRUE, 0,
                               map_y.channels()*map_y.cols*map_y.rows*sizeof(uchar),
                               (void*)(map_y.data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer g_image_map_y ret=%d %d %d %d\n", ret, map_y.channels(), map_y.cols, map_y.rows);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start6 = clock();
#endif
    ret = clEnqueueWriteBuffer(mCQ,
                               mImageMask,
                               CL_TRUE, 0,
                               mask.channels()*mask.cols*mask.rows*sizeof(uchar),
                               (void*)(mask.data),
                               0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed write buffer g_image_mask ret=%d %d %d %d\n", ret, mask.channels(), mask.cols, mask.rows);
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

int CLPano2D::stitch_cl_new_output_buffer(const std::vector<cv::Mat>& side_imgs,
                                const cv::Mat& map_x, const cv::Mat& map_y,
                                const cv::Mat& mask,
                                cv::Mat& image_pano2d)
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

    int size = image_pano2d.channels()*image_pano2d.cols* image_pano2d.rows*sizeof(uchar);
    mImagePano2d = clCreateBuffer (mContext, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
            size,
            NULL,
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_pano2d %d\n", ret);
        return -1;
    }
#if USE_MAP
    mImagePano2dMap = clEnqueueMapBuffer(mCQ, mImagePano2d, CL_TRUE, CL_MAP_READ, 0,
		size,
		0, NULL, NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Map  image_mapy buffer\n");
        return -1;
    }
#endif

    mOutputBufferReady = TRUE;
    printf ("\nAllocation buffer ok\n");
}

void CLPano2D::stitch_cl_delete_output_buffer()
{
    clEnqueueUnmapMemObject(mCQ, mImagePano2d, mImagePano2dMap, 0, NULL, NULL);
    clReleaseMemObject (mImagePano2d);
    mOutputBufferReady = FALSE;
}

int CLPano2D::stitch_cl_new_input_buffer(const std::vector<cv::Mat>& side_imgs,
        const cv::Mat& map_x, const cv::Mat& map_y,
        const cv::Mat& mask,
        cv::Mat& image_pano2d)
{
    cl_int ret;
#if DEBUG_STITCH
    //clock_t start0 = clock();
#endif
    mImageFront = clCreateBuffer (mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            side_imgs[0].channels()*side_imgs[0].cols*side_imgs[0].rows*sizeof(uchar),
            (void*)(side_imgs[0].data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_front %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start1 = clock();
#endif
    mImageRear = clCreateBuffer (mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            side_imgs[1].channels()*side_imgs[1].cols*side_imgs[1].rows*sizeof(uchar),
            (void*)(side_imgs[1].data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_rear %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start2 = clock();
#endif
    mImageLeft = clCreateBuffer (mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            side_imgs[2].channels()*side_imgs[2].cols*side_imgs[2].rows*sizeof(uchar),
            (void*)(side_imgs[2].data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_left %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start3 = clock();
#endif
    mImageRight = clCreateBuffer (mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            side_imgs[3].channels()*side_imgs[3].cols*side_imgs[3].rows*sizeof(uchar),
            (void*)(side_imgs[3].data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_right %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start4 = clock();
#endif
    mImageMask  = clCreateBuffer (mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            mask.channels()*mask.cols*mask.rows*sizeof(uchar),
            (void*)(mask.data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_mask %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start5 = clock();
#endif
    mImageMapX = clCreateBuffer (mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            map_x.channels()*map_x.cols*map_x.rows*sizeof(uchar),
            (void*)(map_x.data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_map_x %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start6 = clock();
#endif
    mImageMapY = clCreateBuffer (mContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            map_y.channels()*map_y.cols*map_y.rows*sizeof(uchar),
            (void*)(map_y.data),
            &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Allocation buffer g_image_map_y %d\n", ret);
        return -1;
    }

#if DEBUG_STITCH
    //clock_t start7 = clock();
#endif

    ret = clSetKernelArg (mKernel, 0, sizeof(cl_mem), &mImageFront);
    ret |= clSetKernelArg (mKernel, 1, sizeof(cl_mem), &mImageRear);
    ret |= clSetKernelArg (mKernel, 2, sizeof(cl_mem), &mImageLeft);
    ret |= clSetKernelArg (mKernel, 3, sizeof(cl_mem), &mImageRight);
    ret |= clSetKernelArg (mKernel, 4, sizeof(int), &(side_imgs[0].cols));
    ret |= clSetKernelArg (mKernel, 5, sizeof(int), &(side_imgs[0].rows));
    ret |= clSetKernelArg (mKernel, 6, sizeof(cl_mem), &mImageMask);
    ret |= clSetKernelArg (mKernel, 7, sizeof(cl_mem), &mImageMapX);
    ret |= clSetKernelArg (mKernel, 8, sizeof(cl_mem), &mImageMapY);
    ret |= clSetKernelArg (mKernel, 9, sizeof(cl_mem), &mImagePano2d);
    ret |= clSetKernelArg (mKernel, 10, sizeof(int), &(image_pano2d.cols));
    ret |= clSetKernelArg (mKernel, 11, sizeof(int), &(image_pano2d.rows));
    if (ret != CL_SUCCESS)
    {
        printf ("\n Failed Set Kernel Arg %d\n", ret);
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
		(start6-start5)/CLOCKS_PER_SEC,
		(start7-start6)/CLOCKS_PER_SEC,
		(start7-start0)/CLOCKS_PER_SEC
		);
*/
#endif
    return 0;
}

void CLPano2D::stitch_cl_delete_input_buffer()
{
    clReleaseMemObject (mImageFront);
    clReleaseMemObject (mImageRear);
    clReleaseMemObject (mImageLeft);
    clReleaseMemObject (mImageRight);
    clReleaseMemObject (mImageMask);
    clReleaseMemObject (mImageMapX);
    clReleaseMemObject (mImageMapY);
}

