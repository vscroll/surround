#ifndef STITCH_CL_H
#define STITCH_CL_H

#include "common.h"
#include <opencv/cv.h>

int stitch_cl_init(char* cl_file, char* cl_kernel_name);

void stitch_cl_uninit();

int stitch_cl_new_pano2d_buffer(int in_side_width, int in_side_height,
                                int out_side_width, int out_side_height,
                                int out_pano2d_width, int  out_pano2d_height);
void stitch_cl_free_pano2d_buffer();

int stitch_cl_2d(const std::vector<cv::Mat>& fishImgs,
                 const cv::Mat& mapX, const cv::Mat& mapY,
                 const cv::Mat& mask,
                 int width, int height,
                 int image_pano2d[VIDEO_PANO2D_RES_Y][VIDEO_PANO2D_RES_X]
                 );

#endif // STITCH_CL_H
