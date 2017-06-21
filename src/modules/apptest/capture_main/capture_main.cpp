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
#include "sourceshmwriteworker.h"

int main (int argc, char **argv)
{
    IConfig* config = new ConfigImpl();
    char cfgPath[1024] = {0};
    if (Util::getAbsolutePath(cfgPath, 1024) < 0)
    {
        std::cout << "capture_main:: path error"
                << std::endl;
        return -1;
    }

    char cfgPathName[1024] = {0};
    sprintf(cfgPathName, "%sconfig.ini", cfgPath);
    if (config->loadFile(cfgPathName) < 0)
    {
        std::cout << "capture_main:: load config error"
                << std::endl;
        return -1;
    }

    int frontChn;
    int rearChn;
    int leftChn;
    int rightChn;
    if (config->getChannelNo(&frontChn, &rearChn, &leftChn, &rightChn) < 0)
    {
        std::cout << "capture_main:: channel no error"
                << std::endl;
        return -1;
    }

    int fps = config->getCaptureFPS();
    if (fps <= 0)
    {
        fps = VIDEO_FPS_15;
    }

	int sinkWidth = config->getSinkWidth();
	int sinkHeight = config->getSinkHeight();
	int cropX = config->getSinkCropX();
	int cropY = config->getSinkCropY();
	int cropWidth = config->getSinkCropWidth();
	int cropHeight = config->getSinkCropHeight();
	int srcWidth = config->getSrcWidth();
	int srcHeight = config->getSrcHeight();
	if (cropX < 0
		|| cropY < 0
		|| cropWidth < 0
		|| cropHeight < 0
		|| (cropX + cropWidth) > CAPTURE_VIDEO_RES_X
		|| (cropY + cropHeight) > CAPTURE_VIDEO_RES_Y
		|| srcWidth < 0
		|| srcHeight < 0
		|| srcWidth > CAPTURE_VIDEO_RES_X
		|| srcHeight > CAPTURE_VIDEO_RES_Y)
	{
        std::cout << "capture_main:: capture config error"
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
        sink[i].crop_x = cropX;
        sink[i].crop_y = cropY;
        sink[i].crop_w = cropWidth;
        sink[i].crop_h = cropHeight;

        // source's pixfmt is same as sink
        source[i].pixfmt = sink[i].pixfmt;
        source[i].width = srcWidth;
        source[i].height = srcHeight;
        source[i].size = V4l2::getVideoSize(source[i].pixfmt, source[i].width, source[i].height);
    }
    if (capture->setCapCapacity(sink, source, VIDEO_CHANNEL_SIZE))
    {
        std::cout << "capture_main::setCapCapacity error"
                << std::endl;
        return -1;        
    }

    // focus source is same as source
    int focusChannelIndex = VIDEO_CHANNEL_FRONT;
    struct cap_src_t focusSource;
    focusSource.pixfmt = source[0].pixfmt;
    focusSource.width = source[0].width;
    focusSource.height = source[0].height;
    focusSource.size = source[0].size;
    capture->setFocusSource(focusChannelIndex, &focusSource);

    if (capture->openDevice(channel, VIDEO_CHANNEL_SIZE) < 0)
    {
        std::cout << "capture_main::openDevice error"
                << std::endl;
        return -1;
    }

    capture->start(fps);

    //focus source
    FocusSourceSHMWriteWorker* focusSourceSHMWriteWorker = new FocusSourceSHMWriteWorker(capture);
    focusSourceSHMWriteWorker->start(fps);

    //4 source
    SourceSHMWriteWorker* sourceSHMWriteWorker[VIDEO_CHANNEL_SIZE] = {NULL};
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        sourceSHMWriteWorker[i] = new SourceSHMWriteWorker(capture, i);
        sourceSHMWriteWorker[i]->start(fps);
    }

#if 0
    //all source
    //AllSourcesSHMWriteWorker* allSourcesSHMWriteWorker = new AllSourcesSHMWriteWorker(capture);
    //allSourcesSHMWriteWorker->start(VIDEO_FPS_15);
#endif

#if 0
    IPanoImage* panoImage = new PanoImageImpl();
    panoImage->init(capture,
            CAPTURE_VIDEO_RES_X, CAPTURE_VIDEO_RES_Y, V4L2_PIX_FMT_YUYV,
            RENDER_VIDEO_RES_PANO_X, RENDER_VIDEO_RES_PANO_Y, V4L2_PIX_FMT_YUYV,
            "/home/root/ckt-demo/PanoConfig.bin", true);
    panoImage->start(VIDEO_FPS_15);
#endif

    // touch screen event
    int eventFd = open("/dev/input/event0", O_RDONLY);
    struct input_event event[64] = {0};

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(eventFd, &fds);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    bool drop = true;

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

            // drop invaild event at the beginning of app
            int rd = read(eventFd, event, sizeof(struct input_event) * 64);
            if (drop)
            {
                drop = false;
                continue;
            }

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

    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        sourceSHMWriteWorker[i]->stop();
        delete sourceSHMWriteWorker[i];
        sourceSHMWriteWorker[i] = NULL;
    }
#if 0
    allSourcesSHMWriteWorker->stop();
    delete allSourcesSHMWriteWorker;
    allSourcesSHMWriteWorker = NULL;
#endif

    capture->closeDevice();
    capture->stop();
    delete capture;
    capture = NULL;

    config->unloadFile();
    delete config;
    config = NULL;

    return 0;
}
