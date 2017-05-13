#ifndef ISIDEIMAGE_H
#define ISIDEIMAGE_H

#include "common.h"

class ISideImage
{
public:
    virtual ~ISideImage() {}
    virtual int init(unsigned int inWidth,
		unsigned int inHeight,
		unsigned int inPixfmt,        
		unsigned int outWidth,
		unsigned int outHeight,
		unsigned int outPixfmt) = 0;
    virtual int start(unsigned int fps) = 0;
    virtual void stop() = 0;
    virtual void queueImage(void* image) = 0;
    virtual void* dequeueImage() = 0;
};

#endif // ISIDEIMAGE_H
