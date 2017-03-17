#ifndef STITCH_ALGORITHM_H
#define STITCH_ALGORITHM_H

#include <opencv/cv.h>
void stitching(const IplImage* front, const IplImage* rear, const IplImage* left, const IplImage* right,
               IplImage** outFull,
               IplImage** outSmall,
               int channel);

#endif // STITCH_ALGORITHM_H
