#ifndef IPANO_H
#define IPANO_H

#include "common.h"

class IPano
{
public:
    virtual ~IPano() {}
    virtual int init(unsigned int inWidth,
		unsigned int inHeight,
		unsigned int inPixfmt,        
		unsigned int panoWidth,
		unsigned int panoHeight,
		unsigned int panoPixfmt,
		char* algoCfgFilePath,
		bool enableOpenCL) = 0;
    virtual int start(unsigned int fps) = 0;
    virtual void stop() = 0;
    virtual void queueImages(surround_images_t* surroundImages) = 0;
    virtual surround_image_t* dequeuePanoImage() = 0;
};

#endif // IPANO_H
