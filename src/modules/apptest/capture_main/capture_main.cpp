#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include "common.h"
#include "controller.h"

int main (int argc, char **argv)
{
    Controller controller;
    if (controller.initConfigModule() < 0)
    {
        goto failed;
    }

    if (controller.startCaptureModule(true) < 0)
    {
        goto failed;
    }

    if (controller.startInputEventModule() < 0)
    {
        goto failed;
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

    while (true)
    {
        sleep(100);
    }

failed:
    controller.stopInputEventModule();
    controller.stopCaptureModule();
    controller.uninitConfigModule();
#if 0
    allSourcesSHMWriteWorker->stop();
    delete allSourcesSHMWriteWorker;
    allSourcesSHMWriteWorker = NULL;
#endif

    return 0;
}
