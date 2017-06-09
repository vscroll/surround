#include "focussourceshmwriteworker.h"
#include "ICapture.h"
#include "imageshm.h"

FocusSourceSHMWriteWorker::FocusSourceSHMWriteWorker(ICapture* capture)
{
    mCapture = capture;
    mImageSHM = new ImageSHM();
    mImageSHM->create((key_t)SHM_FOCUS_SOURCE_ID, SHM_FOCUS_SOURCE_SIZE);
}

FocusSourceSHMWriteWorker::~FocusSourceSHMWriteWorker()
{
    if (NULL != mImageSHM)
    {
        mImageSHM->destroy();
        delete mImageSHM;
        mImageSHM = NULL;
    }
}

void FocusSourceSHMWriteWorker::run()
{
    if (NULL == mCapture
        || NULL == mImageSHM)
    {
        return;
    }

    surround_image_t* focusSource = mCapture->popOneFrame4FocusSource();
    if (NULL != focusSource)
    {
        unsigned int channel = mCapture->getFocusChannelIndex();
#if DEBUG_CAPTURE
        clock_t start = clock();
#endif
        mImageSHM->writeFocusSource(focusSource, channel);
#if DEBUG_CAPTURE
        std::cout << "FocusSourceSHMWriteWorker run: " << (double)(clock()-start)/CLOCKS_PER_SEC
                << " channel:" << channel
                << " width:" << focusSource->info.width
                << " height:" << focusSource->info.height
                << " size:" << focusSource->info.size
                << " timestamp:" << focusSource->timestamp
                << std::endl;
#endif
        delete focusSource;
        focusSource = NULL;
    }
}

class AllSourcesSHMWriteWorker : public WrapThread
{
public:
    AllSourcesSHMWriteWorker(ICapture* capture);
    virtual ~AllSourcesSHMWriteWorker();

public:
    virtual void run();

private:
    ICapture* mCapture;
    ImageSHM* mImageSHM;
    clock_t mLastCallTime;
};

