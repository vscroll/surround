#include "stitch_algorithm.h"
#include "common.h"

extern void CarPano(const std::vector<cv::Mat>& fishImgs,
                    const cv::Mat& map, const cv::Mat& mask,
                    cv::Mat** outPano2D, int outPano2DWidth, int outPano2DHeight,
                    cv::Mat** outSide, int outSideWidth, int outSideHeight, int outSideChannel);

extern void CarPano_cl(const std::vector<cv::Mat>& fishImgs,
                    const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                    cv::Mat** outPano2D, int outPano2DWidth, int outPano2DHeight,
                    cv::Mat** outSide, int outSideWidth, int outSideHeight, int outSideChannel);

void stitching(const void* front, const void* rear, const void* left, const void* right,
               const cv::Mat& map, const cv::Mat& mask,
               void** outPano2D, int outPano2DWidth, int outPano2DHeight,
               void** outSide, int outSideWidth, int outSideHeight, int outSideChannel)
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
                *outPano2D = new cv::Mat(*((cv::Mat*)front));
                *outSide = new cv::Mat(*((cv::Mat*)front));
            }
            break;
        case VIDEO_CHANNEL_REAR:
            {
                *outPano2D = new cv::Mat(*((cv::Mat*)rear));
                *outSide = new cv::Mat(*((cv::Mat*)rear));
            }
            break;
        case VIDEO_CHANNEL_LEFT:
            {
                *outPano2D = new cv::Mat(*((cv::Mat*)left));
                *outSide = new cv::Mat(*((cv::Mat*)left));
            }
            break;
        case VIDEO_CHANNEL_RIGHT:
            {
                *outPano2D = new cv::Mat(*((cv::Mat*)right));
                *outSide = new cv::Mat(*((cv::Mat*)right));
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

    CarPano(fishImgs, map, mask, (cv::Mat**)outPano2D, outPano2DWidth, outPano2DHeight, (cv::Mat**)outSide, outSideWidth, outSideHeight, outSideChannel);

#endif
}

void stitching_cl(const void* front, const void* rear, const void* left, const void* right,
                const cv::Mat& mapX, const cv::Mat& mapY, const cv::Mat& mask,
                void** outPano2D, int outPano2DWidth, int outPano2DHeight,
                void** outSide, int outSideWidth, int outSideHeight, int outSideChannel)
{
    if (NULL == front || NULL == rear || NULL == left || NULL == right)
    {
        return;
    }

    std::vector<cv::Mat> fishImgs;
    cv::Mat matFront(*(cv::Mat*)front);
    cv::Mat matRear(*(cv::Mat*)rear);
    cv::Mat matLeft(*(cv::Mat*)left);
    cv::Mat matRight(*(cv::Mat*)right);

    fishImgs.push_back(matFront);
    fishImgs.push_back(matRear);
    fishImgs.push_back(matLeft);
    fishImgs.push_back(matRight);

    CarPano_cl(fishImgs, mapX, mapY, mask, (cv::Mat**)outPano2D, outPano2DWidth, outPano2DHeight, (cv::Mat**)outSide,  outSideWidth, outSideHeight, outSideChannel);
}
