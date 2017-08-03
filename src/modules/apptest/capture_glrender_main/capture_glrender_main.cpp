#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "common.h"
#include <linux/input.h>

#include "controllergl.h"

int main (int argc, char **argv)
{
    ControllerGL controller;
    if (controller.initConfigModule() < 0)
    {
        return -1;
    }

    if (controller.startCaptureModule(false) < 0)
    {
        controller.uninitConfigModule();
        return -1;
    }

    
    if (controller.startGLRenderModule() < 0)
    {
        controller.stopCaptureModule();
        controller.uninitConfigModule();
        return -1;
    }

    int focusChannelIndex = VIDEO_CHANNEL_FRONT;

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
                        controller.updateFocusChannel(focusChannelIndex);
                        break;
                    }
                }
            }
        }

        usleep(100);
    }

    controller.stopGLRenderModule();
    controller.stopCaptureModule();
    controller.uninitConfigModule();

    return 0;
}
