#include "stitch_algorithm.h"
#include "common.h"

void stitching(const IplImage* front, const IplImage* rear, const IplImage* left, const IplImage* right,
               IplImage** outFull,
               IplImage** outSmall,
               int channel)
{
    if (NULL == front || NULL == rear || NULL == left || NULL == right)
    {
        return;
    }

    switch (channel)
    {
        case VIDEO_CHANNEL_FRONT:
            {
                CvSize dstSize;
                dstSize.width = front->width;
                dstSize.height = front->height;
                *outFull = cvCreateImage(dstSize, front->depth, front->nChannels);
                cvZero(*outFull);
                cvCopy(front, *outFull);

                *outSmall = cvCreateImage(dstSize, front->depth, front->nChannels);
                cvZero(*outSmall);
                cvCopy(front, *outSmall);
            }
            break;
        case VIDEO_CHANNEL_REAR:
            {
                CvSize dstSize;
                dstSize.width = rear->width;
                dstSize.height = rear->height;
                *outFull = cvCreateImage(dstSize, rear->depth, rear->nChannels);
                cvZero(*outFull);
                cvCopy(rear, *outFull);

                *outSmall = cvCreateImage(dstSize, rear->depth, rear->nChannels);
                cvZero(*outSmall);
                cvCopy(rear, *outSmall);
            }
            break;
        case VIDEO_CHANNEL_LEFT:
            {
                CvSize dstSize;
                dstSize.width = left->width;
                dstSize.height = left->height;
                *outFull = cvCreateImage(dstSize, left->depth, left->nChannels);
                cvZero(*outFull);
                cvCopy(left, *outFull);

                *outSmall = cvCreateImage(dstSize, left->depth, left->nChannels);
                cvZero(*outSmall);
                cvCopy(left, *outSmall);
            }
            break;
        case VIDEO_CHANNEL_RIGHT:
            {
                CvSize dstSize;
                dstSize.width = right->width;
                dstSize.height = right->height;
                *outFull = cvCreateImage(dstSize, right->depth, right->nChannels);
                cvZero(*outFull);
                cvCopy(right, *outFull);

                *outSmall = cvCreateImage(dstSize, right->depth, right->nChannels);
                cvZero(*outSmall);
                cvCopy(right, *outSmall);
            }
            break;
        default:
            break;
    }
}
