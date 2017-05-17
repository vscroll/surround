#ifndef PANOIMAGEIMPL_H
#define PANOIMAGEIMPL_H

#include "IPanoImage.h"

class StitchWorker;
class PanoImageImpl : public IPanoImage
{
public:
    PanoImageImpl();
    virtual ~PanoImageImpl();
    virtual int init(
        ICapture* capture,
        unsigned int inWidth,
		unsigned int inHeight,
		unsigned int inPixfmt,        
		unsigned int panoWidth,
		unsigned int panoHeight,
		unsigned int panoPixfmt,
		char* algoCfgFilePath,
		bool enableOpenCL);
    virtual void uninit();
    virtual int start(unsigned int fps);
    virtual void stop();
    virtual void queueImages(surround_images_t* surroundImages);
    virtual surround_image_t* dequeuePanoImage();

private:
    StitchWorker *mWorker;
};

#endif // PANOIMAGEIMPL_H
