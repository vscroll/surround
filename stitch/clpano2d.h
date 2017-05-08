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

    int stitch_cl_2d(const std::vector<cv::Mat>& side_imgs,
                 const cv::Mat& map_x, const cv::Mat& map_y,
                 const cv::Mat& mask,
                 cv::Mat& image_pano2d
                 );
private:
    int stitch_cl_new_pano2d_buffer(const std::vector<cv::Mat>& side_imgs,
                                       const cv::Mat& map_x, const cv::Mat& map_y,
                                       const cv::Mat& mask,
                                       cv::Mat& image_pano2d);
    int stitch_cl_write_pano2d_buffer(const std::vector<cv::Mat>& side_imgs,
                                       const cv::Mat& map_x, const cv::Mat& map_y,
                                       const cv::Mat& mask);
    void stitch_cl_delete_pano2d_buffer();

    int stitch_cl_new_output_buffer(const std::vector<cv::Mat>& side_imgs,
                                       const cv::Mat& map_x, const cv::Mat& map_y,
                                       const cv::Mat& mask,
                                       cv::Mat& image_pano2d);
    void stitch_cl_delete_output_buffer();
    int stitch_cl_new_input_buffer(const std::vector<cv::Mat>& side_imgs,
        const cv::Mat& map_x, const cv::Mat& map_y,
        const cv::Mat& mask,
        cv::Mat& image_pano2d);
    void stitch_cl_delete_input_buffer();
private:
    cl_kernel mKernel;

    cl_mem mImageFront;
    cl_mem mImageRear;
    cl_mem mImageLeft;
    cl_mem mImageRight;
    cl_mem mImageMask;
    cl_mem mImageMapX;
    cl_mem mImageMapY;

    cl_mem mImagePano2d;
    void* mImagePano2dMap;

    int mBufferReady ;
    int mOutputBufferReady;
};

#endif // CLPANO2D_H
