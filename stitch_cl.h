#ifndef STITCH_CL_H
#define STITCH_CL_H

#include "common.h"
#include <opencv/cv.h>

#define CL_HELLOWORLD 0

int stitch_cl_init(char* cl_file, char* cl_kernel_name);

void stitch_cl_uninit();

int stitch_cl_2d(const std::vector<cv::Mat>& side_imgs,
                 const cv::Mat& map_x, const cv::Mat& map_y,
                 const cv::Mat& mask,
                 cv::Mat& image_pano2d
                 );

#if CL_HELLOWORLD
int stitch_cl_new_helloworld_buffer(int width, int height);
int stitch_cl_delete_helloworld_buffer();
int stitch_cl_helloworld(int helloworld_in[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX],
                         int helloworld_out[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX],
                         int width,
                         int height);
#endif

#endif // STITCH_CL_H
