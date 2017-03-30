#ifndef STITCH_ALGORITHM_H
#define STITCH_ALGORITHM_H

#include <opencv/cv.h>

void stitching_init(const std::string config_path, std::vector<cv::Mat>& Maps);

void stitching(const void* front, const void* rear, const void* left, const void* right,
               const std::vector<cv::Mat>& Maps,
               void** outFull,
               void** outSmall,
               int channel);

#endif // STITCH_ALGORITHM_H
