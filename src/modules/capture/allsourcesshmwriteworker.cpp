#include "allsourcesshmwriteworker.h"
#include "ICapture.h"
#include "imageshm.h"

AllSourcesSHMWriteWorker::AllSourcesSHMWriteWorker(ICapture* capture)
{
    mCapture = capture;
    mImageSHM = new ImageSHM();
    mImageSHM->create((key_t)SHM_ALL_SOURCES_ID, SHM_ALL_SOURCES_SIZE);
    mLastCallTime = 0;
}

AllSourcesSHMWriteWorker::~AllSourcesSHMWriteWorker()
{
    if (NULL != mImageSHM)
    {
        mImageSHM->destroy();
        delete mImageSHM;
        mImageSHM = NULL;
    }
}

void AllSourcesSHMWriteWorker::run()
{
#if DEBUG_CAPTURE
    clock_t start = clock();
    double elapsed_to_last = 0;
    if (mLastCallTime != 0)
    {
        elapsed_to_last = (double)(start - mLastCallTime)/CLOCKS_PER_SEC;
    }
    mLastCallTime = start;
#endif

    if (NULL == mCapture
        || NULL == mImageSHM)
    {
        return;
    }

    surround_images_t* sources = mCapture->popOneFrame();
    if (NULL != sources)
    {
#if DEBUG_CAPTURE
        clock_t start = clock();
#endif
        mImageSHM->writeAllSources(sources);
#if DEBUG_CAPTURE
        std::cout << "AllSourcesSHMWriteWorker run: " 
                << " thread id:" << getTID()
                << ", elapsed to last time:" << elapsed_to_last
                << ", elapsed to capture:" << (double)(Util::get_system_milliseconds() - sources->timestamp)/1000
                << ", runtime:" << (double)(clock()-start)/CLOCKS_PER_SEC
                << std::endl;
#endif
        delete sources;
        sources = NULL;
    }
}
