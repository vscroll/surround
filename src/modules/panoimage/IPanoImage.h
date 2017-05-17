#ifndef IPANOIMAGE_H
#define IPANOIMAGE_H

#include "common.h"

class ICapture;
class IPanoImage
{
public:
    virtual ~IPanoImage() {}
    virtual int init(
        ICapture* capture,
        unsigned int inWidth,
		unsigned int inHeight,
		unsigned int inPixfmt,        
		unsigned int panoWidth,
		unsigned int panoHeight,
		unsigned int panoPixfmt,
		char* algoCfgFilePath,
		bool enableOpenCL) = 0;
    virtual void uninit() = 0;
    virtual int start(unsigned int fps) = 0;
    virtual void stop() = 0;
    virtual void queueImages(surround_images_t* surroundImages) = 0;
    virtual surround_image_t* dequeuePanoImage() = 0;
};

#endif // IPANOIMAGE_H
