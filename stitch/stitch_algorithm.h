#ifndef STITCH_ALGORITHM_H
#define STITCH_ALGORITHM_H

#include <opencv/cv.h>

void stitching_init(const std::string config_path, cv::Mat& map, cv::Mat& mask, bool enableOpenCL);

void stitching(const void* front, const void* rear, const void* left, const void* right,
               const cv::Mat& map, const cv::Mat& mask,
               void** outPano2D, int outPano2DWidth, int outPano2DHeight,
               void** outSide, int outSideWidth, int outSideHeight, int outSideChannel);

void stitching_cl(const void* front, const void* rear, const void* left, const void* right,
                const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                void** outPano2D, int outPano2DWidth, int outPano2DHeight,
                void** outSide, int outSideWidth, int outSideHeight, int outSideChannel);


#endif // STITCH_ALGORITHM_H
