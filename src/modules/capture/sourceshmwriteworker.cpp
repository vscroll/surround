#include "sourceshmwriteworker.h"
#include <iostream>
#include "ICapture.h"
#include "imageshm.h"
#include "util.h"

SourceSHMWriteWorker::SourceSHMWriteWorker(ICapture* capture, unsigned int channelIndex)
{
    //pthread_mutex_init(&mMutex, NULL);

    mChannelIndex = channelIndex;
    mCapture = capture;
    mImageSHM = new ImageSHM();
    mImageSHM->create((key_t)(SHM_FRONT_SOURCE_ID + mChannelIndex), SHM_FRONT_SOURCE_SIZE);
    mLastCallTime = 0;
}

SourceSHMWriteWorker::~SourceSHMWriteWorker()
{
    if (NULL != mImageSHM)
    {
        mImageSHM->destroy();
        delete mImageSHM;
        mImageSHM = NULL;
    }
}

void SourceSHMWriteWorker::run()
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

    surround_image_t* source = mCapture->popOneFrame(mChannelIndex);
    if (NULL != source)
    {
#if DEBUG_CAPTURE
        clock_t start = clock();
#endif
        //pthread_mutex_lock(&mMutex);
        mImageSHM->writeSource(source);
        //pthread_mutex_unlock(&mMutex);
#if DEBUG_CAPTURE
        std::cout << "SourceSHMWriteWorker run: " 
                << " thread id:" << getTID()
                << ", elapsed to last time:" << elapsed_to_last
                << ", elapsed to capture:" << (double)(Util::get_system_milliseconds() - source->timestamp)/1000
                << ", runtime:" << (double)(clock()-start)/CLOCKS_PER_SEC
                << ", channel:" << mChannelIndex
                << ", addr:" << source->data
                << std::endl;
#endif
        delete source;
        source = NULL;
    }
}
