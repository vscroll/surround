#include "panosourceshmwriteworker.h"
#include "IPanoImage.h"
#include "imageshm.h"

PanoSourceSHMWriteWorker::PanoSourceSHMWriteWorker(IPanoImage* panoImage)
{
    mPanoImage = panoImage;
    mImageSHM = new ImageSHM();
    mImageSHM->create((key_t)SHM_PANO_SOURCE_ID, SHM_PANO_SOURCE_SIZE);
}

PanoSourceSHMWriteWorker::~PanoSourceSHMWriteWorker()
{
    if (NULL != mImageSHM)
    {
        mImageSHM->destroy();
        delete mImageSHM;
        mImageSHM = NULL;
    }
}

void PanoSourceSHMWriteWorker::run()
{
    if (NULL == mPanoImage
        || NULL == mImageSHM)
    {
        return;
    }

    surround_image_t* source = mPanoImage->dequeuePanoImage();
    if (NULL != source)
    {
#if 0
        clock_t start = clock();
#endif
        mImageSHM->writePanoSources(source);
#if 0
        std::cout << "PanoSourceSHMWriteWorker run: " << (double)(clock()-start)/CLOCKS_PER_SEC
                << " width:" << source->info.width
                << " height:" << source->info.height
                << " size:" << source->info.size
                << " timestamp:" << source->timestamp
                << std::endl;
#endif
        if (NULL != source->data)
        {
            delete (unsigned char*)source->data;
        }

        delete source;
        source = NULL;
    }
}
