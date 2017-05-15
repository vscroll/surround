#include "controller.h"
#include "common.h"
#include "ICapture.h"
#include "captureimpl.h"
#include "IPanoImage.h"
#include "panoimageimpl.h"
#include "imageshm.h"
#include <opencv/cv.h>

Controller::Controller()
{
    mCapture = NULL;
    mPanoImage = NULL;

    mCurChannelIndex = VIDEO_CHANNEL_FRONT;

    mSideSHM = NULL;
    mPanoSHM = NULL;

    mPanoSHMWorker = NULL;
}

Controller::~Controller()
{
}

void Controller::initCaptureModule(unsigned int channel[], unsigned int channelNum,
		struct cap_sink_t sink[], struct cap_src_t source[])
{
    if (NULL == mCapture)
    {
        mCapture = new CaptureImpl();
    }
    mCapture->setCapCapacity(sink, source, channelNum);
    struct cap_src_t focusSource;
    focusSource.pixfmt = sink[0].pixfmt;
    focusSource.width = sink[0].width;
    focusSource.height = sink[0].height;
    focusSource.size = sink[0].size;
    //mCapture->setFocusSource(0, &focusSource);
    mCapture->openDevice(channel, channelNum);
}

void Controller::initPanoImageModule(unsigned int inWidth,
            unsigned int inHeight,
            unsigned int inPixfmt,
            unsigned int panoWidth,
            unsigned int panoHeight,
            unsigned int panoPixfmt,
            char* algoCfgFilePath,
            bool enableOpenCL)
{
    if (NULL == mPanoImage)
    {
        mPanoImage = new PanoImageImpl();
    }

    mPanoImage->init(inWidth, inHeight, inPixfmt, panoWidth, panoHeight, panoPixfmt, algoCfgFilePath, enableOpenCL);
}

void Controller::initSideImageModule(unsigned int curChannelIndex,
            unsigned int outWidth,
            unsigned int outHeight,
            unsigned int outPixfmt)
{
    if (curChannelIndex < VIDEO_CHANNEL_SIZE)
    {
        mCurChannelIndex = curChannelIndex;
    }
}

void Controller::uninitModules()
{
    if (NULL != mCapture)
    {
        delete mCapture;
        mCapture = NULL;
    }

    if (NULL != mPanoImage)
    {
        delete mPanoImage;
        mPanoImage = NULL;
    }
}

void Controller::startModules(unsigned int fps)
{
   if (NULL != mCapture)
   {
        mCapture->start(fps);
   }

   if (NULL != mPanoImage)
   {
        mPanoImage->start(fps);
   }
}

void Controller::stopModules()
{
    if (NULL != mCapture)
    {
        mCapture->stop();
        mCapture->closeDevice();
    }

    if (NULL != mPanoImage)
    {
        mPanoImage->stop();
    }
}

void Controller::startLoop(unsigned int freq)
{
    mSideSHM = new ImageSHM();
    mSideSHM->create((key_t)SHM_SIDE_ID, SHM_SIDE_SIZE);

    mPanoSHM = new ImageSHM();
    mPanoSHM->create((key_t)SHM_PANO2D_ID, SHM_PANO2D_SIZE);

    start(1000/freq);
}

void Controller::stopLoop()
{
    if (NULL != mSideSHM)
    {
        mSideSHM->destroy();
        delete mSideSHM;
        mSideSHM = NULL;
    }

    if (NULL != mPanoSHM)
    {
        mPanoSHM->destroy();
        delete mPanoSHM;
        mPanoSHM = NULL;
    }
}

void Controller::run()
{
    if (NULL == mCapture
        || NULL == mPanoImage)
    {
        return;
    }

    if (NULL == mSideSHM
        || NULL == mPanoSHM)
    {
        return;
    }

    surround_image_t* sideImage = mCapture->popOneFrame4FocusSource();
    if (NULL != sideImage)
    {
		struct image_shm_header_t header = {};
		header.width = sideImage->info.width;
		header.height = sideImage->info.height;
		header.pixfmt = sideImage->info.pixfmt;
		header.size = sideImage->info.size;
		header.timestamp = sideImage->timestamp;
        unsigned char* frame = (unsigned char*)sideImage->data;
        double start = clock();
        if (NULL != frame)
        {
            //mSideSHM->writeImage(&header, frame, header.size);
            delete frame;
        }
#if 0
        std::cout << " side shm write time: " << (clock()-start)/CLOCKS_PER_SEC
                << " width:" << header.width
                << " height:" << header.height
                << " size:" << header.size
                << " timestamp:" << header.timestamp
                << std::endl;
#endif
    }


    surround_images_t* surroundImages = mCapture->popOneFrame();
    if (NULL == surroundImages)
    {
        return;
    }

    //Side
#if 0
    surround_image_t* sideImage = &(surroundImages->frame[mCurChannelIndex]);
    if (NULL != sideImage)
    {
		struct image_shm_header_t header = {};
		header.width = sideImage->info.width;
		header.height = sideImage->info.height;
		header.pixfmt = sideImage->info.pixfmt;
		header.size = sideImage->info.size;
		header.timestamp = sideImage->timestamp;
        cv::Mat* frame = (cv::Mat*)sideImage->data;
        double start = clock();
        if (NULL != frame)
        {
            //mSideSHM->writeImage(&header, (unsigned char*)frame->data, header.size);
        }
#if 0
        std::cout << " side shm write time: " << (clock()-start)/CLOCKS_PER_SEC
                << " width:" << header.width
                << " height:" << header.height
                << " size:" << header.size
                << " timestamp:" << header.timestamp
                << std::endl;
#endif
    }
#endif

    mPanoImage->queueImages(surroundImages);

    //Pano
    surround_image_t* surroundImage = mPanoImage->dequeuePanoImage();
    if (NULL != surroundImage)
    {
		struct image_shm_header_t header = {};
		header.width = surroundImage->info.width;
		header.height = surroundImage->info.height;
		header.pixfmt = surroundImage->info.pixfmt;
		header.size = surroundImage->info.size;
		header.timestamp = surroundImage->timestamp;
        cv::Mat* frame = (cv::Mat*)surroundImage->data;
        double start = clock();
        if (NULL != frame)
        {
            //mPanoSHM->writeImage(&header, (unsigned char*)frame->data, header.size);
            delete frame;
        }
#if 0
        std::cout << " pano shm write time: " << (clock()-start)/CLOCKS_PER_SEC
                << " width:" << header.width
                << " height:" << header.height
                << " size:" << header.size
                << " timestamp:" << header.timestamp
                << std::endl;
#endif

        delete surroundImage;
    } 
}
