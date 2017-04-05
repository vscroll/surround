#ifndef STITCH_ALGORITHM_H
#define STITCH_ALGORITHM_H

#include <opencv/cv.h>

void stitching_init(const std::string config_path, cv::Mat& Maps, cv::Mat& Mask);

void stitching(const void* front, const void* rear, const void* left, const void* right,
               const cv::Mat& Map,
               const cv::Mat& Mask,
               void** outFull,
               int full_width,
               int full_height,
               void** outSmall,
               int channel);

#endif // STITCH_ALGORITHM_H
