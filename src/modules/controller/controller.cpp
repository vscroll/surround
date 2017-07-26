#include "controller.h"
#include <iostream>
#include "common.h"
#include "IConfig.h"
#include "configimpl.h"
#include "ICapture.h"
#include "captureimpl.h"
#include "capture1impl.h"
#include "IPanoImage.h"
#include "panoimageimpl.h"
#include "util.h"
#include "v4l2.h"
#include "focussourceshmwriteworker.h"
#include "sourceshmwriteworker.h"
#include "panosourceshmwriteworker.h"

Controller::Controller()
{
    mConfig = NULL;
    mCapture = NULL;
    mPanoImage = NULL;
    mFocusSourceSHMWriteWorker = NULL;
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mSourceSHMWriteWorker[i] = NULL;
    }

    mPanoSourceSHMWriteWorker = NULL;
}

Controller::~Controller()
{
}

int Controller::initConfigModule()
{
    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    char cfgPathName[1024] = {0};
    sprintf(cfgPathName, "%sconfig.ini", procPath);
    mConfig = new ConfigImpl();
    if (mConfig->loadFromFile(cfgPathName) < 0)
    {
        delete mConfig;
        mConfig = NULL;
        return -1;
    }

    return 0;
}

void Controller::uninitConfigModule()
{
    if (NULL != mConfig)
    {
        delete mConfig;
        mConfig = NULL;
    }
}

int Controller::startCaptureModule()
{
    if (NULL == mConfig)
    {
        return -1;
    }

    //capture
    int frontChn;
    int rearChn;
    int leftChn;
    int rightChn;
    if (mConfig->getChannelNo(&frontChn, &rearChn, &leftChn, &rightChn) < 0)
    {    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }
        return -1;
    }

    int captureFPS = mConfig->getCaptureFPS();
    if (captureFPS <= 0)
    {
        captureFPS = VIDEO_FPS_15;
    }

	//sink
	int sinkWidth = mConfig->getSinkWidth();
	int sinkHeight = mConfig->getSinkHeight();
	if (sinkWidth < 0
		|| sinkHeight < 0
		|| sinkWidth > CAPTURE_VIDEO_RES_X
		|| sinkHeight > CAPTURE_VIDEO_RES_Y)
	{
	    std::cout << "sink config error"
	            << std::endl;
		return -1;
	}
    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }
	//crop
	int cropX[VIDEO_CHANNEL_SIZE];
	int cropY[VIDEO_CHANNEL_SIZE];
	int cropWidth[VIDEO_CHANNEL_SIZE];
	int cropHeight[VIDEO_CHANNEL_SIZE];
	for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
	{
		cropX[i] = mConfig->getSinkCropX(i);
		cropY[i] = mConfig->getSinkCropY(i);
		cropWidth[i] = mConfig->getSinkCropWidth(i);
		cropHeight[i] = mConfig->getSinkCropHeight(i);
		if (cropX[i] < 0
			|| cropY[i] < 0
			|| cropWidth[i] < 0
			|| cropHeight[i] < 0
			|| (cropX[i] + cropWidth[i]) > CAPTURE_VIDEO_RES_X
			|| (cropY[i] + cropHeight[i]) > CAPTURE_VIDEO_RES_Y)
		{
		    std::cout << "sink crop config error"
		            << std::endl;
			return -1;
		}
	}

	int focusSinkCropX = mConfig->getFocusSinkCropX();
	int focusSinkCropY = mConfig->getFocusSinkCropY();
	int focusSinkCropWidth = mConfig->getFocusSinkCropWidth();
	int focusSinkCropHeight = mConfig->getFocusSinkCropHeight();
	if (focusSinkCropX < 0
		|| focusSinkCropY < 0
		|| focusSinkCropX > CAPTURE_VIDEO_RES_X
		|| focusSinkCropY > CAPTURE_VIDEO_RES_Y
		|| focusSinkCropWidth < 0
		|| focusSinkCropHeight < 0
		|| focusSinkCropWidth > CAPTURE_VIDEO_RES_X
		|| focusSinkCropHeight > CAPTURE_VIDEO_RES_Y)
	{
		std::cout << "focus sink crop config error"
				<< std::endl;
		return -1;
	}

	//source
	int srcWidth[VIDEO_CHANNEL_SIZE];
	int srcHeight[VIDEO_CHANNEL_SIZE];
	for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
	{
		srcWidth[i] = mConfig->getSourceWidth(i);
		srcHeight[i] = mConfig->getSourceHeight(i);
		if (srcWidth[i] < 0
			|| srcHeight[i] < 0
			|| srcWidth[i] > CAPTURE_VIDEO_RES_X
			|| srcHeight[i] > CAPTURE_VIDEO_RES_Y)
		{
		    std::cout << "source config error"
		            << std::endl;
			return -1;
		}
	}

	int focusSrcWidth = mConfig->getFocusSourceWidth();
	int focusSrcHeight = mConfig->getFocusSourceHeight();
	if (focusSrcWidth < 0
		|| focusSrcHeight < 0
		|| focusSrcWidth > CAPTURE_VIDEO_RES_X
		|| focusSrcHeight > CAPTURE_VIDEO_RES_Y)
	{
		std::cout << "focus source config error"
				<< std::endl;
		return -1;
	}

#if 0
    mCapture = new CaptureImpl();    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {#include "panosourceshmwriteworker.h"
        return -1;
    }
#else
    mCapture = new Capture1Impl(VIDEO_CHANNEL_SIZE);
#endif

    unsigned int channel[VIDEO_CHANNEL_SIZE] = {frontChn,rearChn,leftChn,rightChn};
    struct cap_sink_t sink[VIDEO_CHANNEL_SIZE];
    struct cap_src_t source[VIDEO_CHANNEL_SIZE];
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        sink[i].pixfmt = V4L2_PIX_FMT_UYVY;
        sink[i].width = CAPTURE_VIDEO_RES_X;
        sink[i].height = CAPTURE_VIDEO_RES_Y;
        sink[i].size = V4l2::getVideoSize(sink[i].pixfmt, sink[i].width, sink[i].height);
        sink[i].crop_x = cropX[i];
        sink[i].crop_y = cropY[i];
        sink[i].crop_w = cropWidth[i];
        sink[i].crop_h = cropHeight[i];

        // source's pixfmt is same as sink
        source[i].pixfmt = sink[i].pixfmt;
        source[i].width = srcWidth[i];
        source[i].height = srcHeight[i];
        source[i].size = V4l2::getVideoSize(source[i].pixfmt, source[i].width, source[i].height);
    }
    if (mCapture->setCapCapacity(sink, source, VIDEO_CHANNEL_SIZE))
    {
        std::cout << "capture_main::setCapCapacity error"
                << std::endl;
        delete mCapture;
        mCapture = NULL;
        return -1;        
    }

    // focus source's pixfmt is same as source
    int focusChannelIndex = VIDEO_CHANNEL_FRONT;
    struct cap_src_t focusSource;
    focusSource.pixfmt = source[0].pixfmt;
    focusSource.width = focusSrcWidth;
    focusSource.height = focusSrcHeight;
    focusSource.size = V4l2::getVideoSize(focusSource.pixfmt, focusSource.width, focusSource.height);
    mCapture->setFocusSource(focusChannelIndex, &focusSource);

    if (mCapture->openDevice(channel, VIDEO_CHANNEL_SIZE) < 0)
    {
        std::cout << "capture_main::openDevice error"
                << std::endl;
        delete mCapture;
        mCapture = NULL;
        return -1;
    }

    mCapture->start(captureFPS);

    int accelPolicy = mConfig->getAccelPolicy();

    //focus source
    mFocusSourceSHMWriteWorker = new FocusSourceSHMWriteWorker(mCapture);
    mFocusSourceSHMWriteWorker->start(captureFPS);

    if (accelPolicy != ACCEL_POLICY_OPENCL_RENDER)
    {
        //4 source
        for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            mSourceSHMWriteWorker[i] = new SourceSHMWriteWorker(mCapture, i);
            mSourceSHMWriteWorker[i]->start(captureFPS);
        }
    }

    return 0;
}

void Controller::stopCaptureModule()
{
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mSourceSHMWriteWorker[i]->stop();
        delete mSourceSHMWriteWorker[i];
        mSourceSHMWriteWorker[i] = NULL;
    }

    if (NULL != mFocusSourceSHMWriteWorker)
    {
        mFocusSourceSHMWriteWorker->stop();
        delete mFocusSourceSHMWriteWorker;
        mFocusSourceSHMWriteWorker = NULL;
    }

    if (NULL != mCapture)
    {
        mCapture->stop();
        delete mCapture;
        mCapture = NULL;
    }    
}

void Controller::updateFocusChannel(int channelIndex)
{
    if (NULL != mCapture)
    {
        mCapture->setFocusSource(channelIndex, NULL);
    }
}

int Controller::startPanoImageModule()
{
    if (NULL == mConfig
        || NULL == mCapture)
    {
        return -1;
    }

    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    char algoCfgPathName[1024] = {0};
    sprintf(algoCfgPathName, "%sFishToPanoYUV.xml", procPath);

    int accelPolicy = mConfig->getAccelPolicy();

    int stitchFPS = mConfig->getStitchFPS();
    if (stitchFPS <= 0)
    {
        stitchFPS = VIDEO_FPS_15;
    }

    mPanoImage = new PanoImageImpl();
    mPanoImage->init(mCapture,
            CAPTURE_VIDEO_RES_X, CAPTURE_VIDEO_RES_Y, V4L2_PIX_FMT_UYVY,
            RENDER_VIDEO_RES_PANO_X, RENDER_VIDEO_RES_PANO_Y, V4L2_PIX_FMT_UYVY,
            algoCfgPathName, accelPolicy);
    mPanoImage->start(stitchFPS);

    if (accelPolicy != ACCEL_POLICY_OPENCL_RENDER)
    {
        mPanoSourceSHMWriteWorker = new PanoSourceSHMWriteWorker(mPanoImage);
        mPanoSourceSHMWriteWorker->start(stitchFPS);
    }

    return 0;
}

void Controller::stopPanoImageModule()
{
    if (NULL != mPanoSourceSHMWriteWorker)
    {
        mPanoSourceSHMWriteWorker->stop();
        delete mPanoSourceSHMWriteWorker;
        mPanoSourceSHMWriteWorker = NULL;
    }

    if (NULL != mPanoImage)
    {
        mPanoImage->stop();
        delete mPanoImage;
        mPanoImage = NULL;
    }  
}
