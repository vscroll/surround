#ifndef PANOSOURCESHMWRITEWORKER_H
#define PANOSOURCESHMWRITEWORKER_H

#include "wrap_thread.h"

class IPanoImage;
class ImageSHM;
class PanoSourceSHMWriteWorker : public WrapThread
{
public:
    PanoSourceSHMWriteWorker(IPanoImage* panoImage);
    virtual ~PanoSourceSHMWriteWorker();

public:
    virtual void run();

private:
    IPanoImage* mPanoImage;
    ImageSHM* mImageSHM;
};

#endif //PANOSOURCESHMWRITEWORKER_H
