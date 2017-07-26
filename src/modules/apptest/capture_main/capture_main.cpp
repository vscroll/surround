#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "common.h"
#include <linux/input.h>
#include "controller.h"

int main (int argc, char **argv)
{
    Controller controller;
    if (controller.initConfigModule() < 0)
    {
        return -1;
    }

    if (controller.startCaptureModule() < 0)
    {
        controller.uninitConfigModule();
        return -1;
    }

    int focusChannelIndex = VIDEO_CHANNEL_FRONT;

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
                        controller.updateFocusChannel(focusChannelIndex);
                        break;
                    }
                }
            }
        }

        usleep(100);
    }

    controller.stopCaptureModule();
    controller.uninitConfigModule();
#if 0
    allSourcesSHMWriteWorker->stop();
    delete allSourcesSHMWriteWorker;
    allSourcesSHMWriteWorker = NULL;
#endif

    return 0;
}
