#ifndef PANOIMPL_H
#define PANOIMPL_H

#include "IPano.h"

class StitchWorker;
class PanoImpl : public IPano
{
public:
    PanoImpl();
    virtual ~PanoImpl();
    virtual int init(unsigned int inWidth,
		unsigned int inHeight,
		unsigned int inPixfmt,        
		unsigned int panoWidth,
		unsigned int panoHeight,
		unsigned int panoPixfmt,
		char* algoCfgFilePath,
		bool enableOpenCL);
    virtual int start(unsigned int fps);
    virtual void stop();
    virtual void queueImages(surround_images_t* surroundImages);
    virtual surround_image_t* dequeuePanoImage();

private:
    StitchWorker *mWorker;
};

#endif // PANOIMPL_H
