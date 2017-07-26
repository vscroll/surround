#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "common.h"
#include "ICapture.h"
#include "captureimpl.h"
#include "capture1impl.h"
#include "imageshm.h"
#include "util.h"
#include "v4l2.h"
#include <linux/input.h>
#include "IPanoImage.h"
#include "panoimageimpl.h"
#include "wrap_thread.h"
#include "IConfig.h"
#include "configimpl.h"

#include "focussourceshmwriteworker.h"
#include "panosourceshmwriteworker.h"

int main (int argc, char **argv)
{
    IConfig* config = new ConfigImpl();

    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    char cfgPathName[1024] = {0};
    sprintf(cfgPathName, "%sconfig.ini", procPath);
    if (config->loadFromFile(cfgPathName) < 0)
    {
        return -1;
    }

    //capture
    int frontChn;
    int rearChn;
    int leftChn;
    int rightChn;
    if (config->getChannelNo(&frontChn, &rearChn, &leftChn, &rightChn) < 0)
    {
        return -1;
    }

    int captureFPS = config->getCaptureFPS();
    if (captureFPS <= 0)
    {
        captureFPS = VIDEO_FPS_15;
    }

	//sink
	int sinkWidth = config->getSinkWidth();
	int sinkHeight = config->getSinkHeight();
	if (sinkWidth < 0
		|| sinkHeight < 0
		|| sinkWidth > CAPTURE_VIDEO_RES_X
		|| sinkHeight > CAPTURE_VIDEO_RES_Y)
	{
	    std::cout << "capture_main:: sink config error"
	            << std::endl;
		return -1;
	}

	//crop
	int cropX[VIDEO_CHANNEL_SIZE];
	int cropY[VIDEO_CHANNEL_SIZE];
	int cropWidth[VIDEO_CHANNEL_SIZE];
	int cropHeight[VIDEO_CHANNEL_SIZE];
	for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
	{
		cropX[i] = config->getSinkCropX(i);
		cropY[i] = config->getSinkCropY(i);
		cropWidth[i] = config->getSinkCropWidth(i);
		cropHeight[i] = config->getSinkCropHeight(i);
		if (cropX[i] < 0
			|| cropY[i] < 0
			|| cropWidth[i] < 0
			|| cropHeight[i] < 0
			|| (cropX[i] + cropWidth[i]) > CAPTURE_VIDEO_RES_X
			|| (cropY[i] + cropHeight[i]) > CAPTURE_VIDEO_RES_Y)
		{
		    std::cout << "capture_main:: sink crop config error"
		            << std::endl;
			return -1;
		}
	}

	int focusSinkCropX = config->getFocusSinkCropX();
	int focusSinkCropY = config->getFocusSinkCropY();
	int focusSinkCropWidth = config->getFocusSinkCropWidth();
	int focusSinkCropHeight = config->getFocusSinkCropHeight();
	if (focusSinkCropX < 0
		|| focusSinkCropY < 0
		|| focusSinkCropX > CAPTURE_VIDEO_RES_X
		|| focusSinkCropY > CAPTURE_VIDEO_RES_Y
		|| focusSinkCropWidth < 0
		|| focusSinkCropHeight < 0
		|| focusSinkCropWidth > CAPTURE_VIDEO_RES_X
		|| focusSinkCropHeight > CAPTURE_VIDEO_RES_Y)
	{
		std::cout << "capture_main:: focus sink crop config error"
				<< std::endl;
		return -1;
	}

	//source
	int srcWidth[VIDEO_CHANNEL_SIZE];
	int srcHeight[VIDEO_CHANNEL_SIZE];
	for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
	{
		srcWidth[i] = config->getSourceWidth(i);
		srcHeight[i] = config->getSourceHeight(i);
		if (srcWidth[i] < 0
			|| srcHeight[i] < 0
			|| srcWidth[i] > CAPTURE_VIDEO_RES_X
			|| srcHeight[i] > CAPTURE_VIDEO_RES_Y)
		{
		    std::cout << "capture_main:: source config error"
		            << std::endl;
			return -1;
		}
	}

	int focusSrcWidth = config->getFocusSourceWidth();
	int focusSrcHeight = config->getFocusSourceHeight();
	if (focusSrcWidth < 0
		|| focusSrcHeight < 0
		|| focusSrcWidth > CAPTURE_VIDEO_RES_X
		|| focusSrcHeight > CAPTURE_VIDEO_RES_Y)
	{
		std::cout << "capture_main:: focus source config error"
				<< std::endl;
		return -1;
	}
#if 0
    ICapture* capture = new CaptureImpl();
#else
    ICapture* capture = new Capture1Impl(VIDEO_CHANNEL_SIZE);
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
    if (capture->setCapCapacity(sink, source, VIDEO_CHANNEL_SIZE))
    {
        std::cout << "capture_main::setCapCapacity error"
                << std::endl;
        return -1;        
    }

    // focus source's pixfmt is same as source
    int focusChannelIndex = VIDEO_CHANNEL_FRONT;
    struct cap_src_t focusSource;
    focusSource.pixfmt = source[0].pixfmt;
    focusSource.width = focusSrcWidth;
    focusSource.height = focusSrcHeight;
    focusSource.size = V4l2::getVideoSize(focusSource.pixfmt, focusSource.width, focusSource.height);
    capture->setFocusSource(focusChannelIndex, &focusSource);

    if (capture->openDevice(channel, VIDEO_CHANNEL_SIZE) < 0)
    {
        std::cout << "capture_main::openDevice error"
                << std::endl;
        return -1;
    }

    capture->start(captureFPS);

    //focus source
    FocusSourceSHMWriteWorker* focusSourceSHMWriteWorker = new FocusSourceSHMWriteWorker(capture);
    focusSourceSHMWriteWorker->start(captureFPS);


    //pano
    char algoCfgPathName[1024] = {0};
    sprintf(algoCfgPathName, "%sFishToPanoYUV.xml", procPath);

    int accelPolicy = config->getAccelPolicy();

    int stitchFPS = config->getStitchFPS();
    if (stitchFPS <= 0)
    {
        stitchFPS = VIDEO_FPS_15;
    }

    IPanoImage* panoImage = new PanoImageImpl();
    panoImage->init(capture,
            CAPTURE_VIDEO_RES_X, CAPTURE_VIDEO_RES_Y, V4L2_PIX_FMT_UYVY,
            RENDER_VIDEO_RES_PANO_X, RENDER_VIDEO_RES_PANO_Y, V4L2_PIX_FMT_UYVY,
            algoCfgPathName, accelPolicy);
    panoImage->start(stitchFPS);

    PanoSourceSHMWriteWorker* panoSourceSHMWriteWorker = NULL;
    if (accelPolicy == ACCEL_POLICY_OPENCL_RENDER)
    {
        panoSourceSHMWriteWorker = new PanoSourceSHMWriteWorker(panoImage);
        panoSourceSHMWriteWorker->start(stitchFPS);
    }

    // touch screen event
    int eventFd = open("/dev/input/event0", O_RDONLY);
    struct input_event event[64] = {0};

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(eventFd, &fds);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while (true)
    {
        //touch screen to switch focus channel
        if (eventFd > 0)
        {
            int r = select (eventFd + 1, &fds, NULL, NULL, &tv);
            if (-1 == r) {
                usleep(100);
            }

            if (0 == r) {
                usleep(100);
            }

            int rd = read(eventFd, event, sizeof(struct input_event) * 64);
            if (rd >= (int) sizeof(struct input_event))
            {
                for (int i = 0; i < rd / sizeof(struct input_event); i++)
                {
                    if (event[i].type == EV_KEY
                            && event[i].code == BTN_TOUCH
                            && event[i].value == 0)
                    {
                        focusChannelIndex++;
                        if (focusChannelIndex >= VIDEO_CHANNEL_SIZE)
                        {
                            focusChannelIndex = VIDEO_CHANNEL_FRONT;
                        }
                        capture->setFocusSource(focusChannelIndex, &focusSource);
                        break;
                    }
                }
            }
        }

        usleep(100);
    }

    focusSourceSHMWriteWorker->stop();
    delete focusSourceSHMWriteWorker;
    focusSourceSHMWriteWorker = NULL;

    if (NULL != panoSourceSHMWriteWorker)
    {
        panoSourceSHMWriteWorker->stop();
        delete panoSourceSHMWriteWorker;
        panoSourceSHMWriteWorker = NULL;
    }

    panoImage->stop();
    delete panoImage;
    panoImage = NULL;

    capture->closeDevice();
    capture->stop();
    delete capture;
    capture = NULL;

    delete config;
    config = NULL;

    return 0;
}
