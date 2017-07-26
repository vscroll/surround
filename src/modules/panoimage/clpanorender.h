#ifndef CLPANORENDER_H
#define CLPANORENDER_H

#include "clpano2d.h"
#include "common.h"
#include <CL/cl.h>
#include <opencv/cv.h>

class CLPanoRender : public CLPano2D
{
public:
    CLPanoRender();
    virtual ~CLPanoRender();

public:
    virtual int stitch(surround_image_t* sideImage[],
    	cv::Mat* lookupTab[],
    	cv::Mat& mask,
    	cv::Mat& weight,
        unsigned int panoWidth,
        unsigned int panoHeight,
        unsigned int panoSize,
        unsigned char* panoImage);
};

#endif // CLPANORENDER_H
