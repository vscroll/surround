#include "stitch_algorithm.h"
#include "common.h"

extern void CarPano(const std::vector<cv::Mat>& fishImgs, const std::vector<cv::Mat>& Maps, const int Channel, cv::Mat** Pano2D, cv::Mat** SideImg);


static int STITCH_CHANNEL[] = {1,3,0,2};
void stitching(const void* front, const void* rear, const void* left, const void* right,
               const std::vector<cv::Mat>& Maps,
               void** outFull,
               void** outSmall,
               int channel)
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
#if DATA_TYPE_IPLIMAGE
                CvSize dstSize;
                IplImage* image = (IplImage*)front;
                dstSize.width = image->width;
                dstSize.height = image->height;
                *outFull = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outFull));
                cvCopy(front, (IplImage*)(*outFull));

                *outSmall = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outSmall));
                cvCopy(image, (IplImage*)(*outSmall));
#else
                *outFull = new cv::Mat(*((cv::Mat*)front));
                *outSmall = new cv::Mat(*((cv::Mat*)front));
#endif
            }
            break;
        case VIDEO_CHANNEL_REAR:
            {
#if DATA_TYPE_IPLIMAGE
                CvSize dstSize;
                IplImage* image = (IplImage*)rear;
                dstSize.width = image->width;
                dstSize.height = image->height;
                *outFull = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outFull));
                cvCopy(front, (IplImage*)(*outFull));

                *outSmall = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outSmall));
                cvCopy(image, (IplImage*)(*outSmall));
#else
                *outFull = new cv::Mat(*((cv::Mat*)rear));
                *outSmall = new cv::Mat(*((cv::Mat*)rear));
#endif
            }
            break;
        case VIDEO_CHANNEL_LEFT:
            {
#if DATA_TYPE_IPLIMAGE
                CvSize dstSize;
                IplImage* image = (IplImage*)left;
                dstSize.width = image->width;
                dstSize.height = image->height;
                *outFull = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outFull));
                cvCopy(front, (IplImage*)(*outFull));

                *outSmall = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outSmall));
                cvCopy(image, (IplImage*)(*outSmall));
#else
                *outFull = new cv::Mat(*((cv::Mat*)left));
                *outSmall = new cv::Mat(*((cv::Mat*)left));
#endif
            }
            break;
        case VIDEO_CHANNEL_RIGHT:
            {
#if DATA_TYPE_IPLIMAGE
                CvSize dstSize;
                IplImage* image = (IplImage*)right;
                dstSize.width = image->width;
                dstSize.height = image->height;
                *outFull = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outFull));
                cvCopy(front, (IplImage*)(*outFull));

                *outSmall = cvCreateImage(dstSize, image->depth, image->nChannels);
                cvZero((IplImage*)(*outSmall));
                cvCopy(image, (IplImage*)(*outSmall));
#else
                *outFull = new cv::Mat(*((cv::Mat*)right));
                *outSmall = new cv::Mat(*((cv::Mat*)right));
#endif
            }
            break;
        default:
            break;
    }

#else

#if DATA_TYPE_IPLIMAGE
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

    int stitch_channel = STITCH_CHANNEL[channel];
    CarPano(fishImgs, Maps, stitch_channel, (cv::Mat**)outFull, (cv::Mat**)outSmall);
#endif

#endif
}
