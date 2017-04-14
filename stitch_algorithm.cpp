#include "stitch_algorithm.h"
#include "common.h"

extern void CarPano(const std::vector<cv::Mat>& fishImgs, const cv::Mat& Map, const cv::Mat& Mask, cv::Mat** Pano2D, int full_width,
                    int full_height, cv::Mat** SideImg, int side_channel);

void stitching(const void* front, const void* rear, const void* left, const void* right,
               const cv::Mat& Map,
               const cv::Mat& Mask,
               void** outFull,
               int full_width,
               int full_height,
               void** outSmall,
               int side_channel)
{
    if (NULL == front || NULL == rear || NULL == left || NULL == right)
    {
        return;
    }

#if DATA_FAKE

    switch (channel)
    {
        case VIDEO_CHANNEL_FRONT:
            {
                *outFull = new cv::Mat(*((cv::Mat*)front));
                *outSmall = new cv::Mat(*((cv::Mat*)front));
            }
            break;
        case VIDEO_CHANNEL_REAR:
            {
                *outFull = new cv::Mat(*((cv::Mat*)rear));
                *outSmall = new cv::Mat(*((cv::Mat*)rear));
            }
            break;
        case VIDEO_CHANNEL_LEFT:
            {
                *outFull = new cv::Mat(*((cv::Mat*)left));
                *outSmall = new cv::Mat(*((cv::Mat*)left));
            }
            break;
        case VIDEO_CHANNEL_RIGHT:
            {
                *outFull = new cv::Mat(*((cv::Mat*)right));
                *outSmall = new cv::Mat(*((cv::Mat*)right));
            }
            break;
        default:
            break;
    }

#else

    std::vector<cv::Mat> fishImgs;
    cv::Mat matFront(*(cv::Mat*)front);
    cv::Mat matRear(*(cv::Mat*)rear);
    cv::Mat matLeft(*(cv::Mat*)left);
    cv::Mat matRight(*(cv::Mat*)right);

    fishImgs.push_back(matFront);
    fishImgs.push_back(matRear);
    fishImgs.push_back(matLeft);
    fishImgs.push_back(matRight);

    CarPano(fishImgs, Map, Mask, (cv::Mat**)outFull, full_width, full_height, (cv::Mat**)outSmall, side_channel);

#endif
}
