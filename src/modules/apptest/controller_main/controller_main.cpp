// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "controller.h"

#define USE_YUV4PANO 1

int main (int argc, char **argv)
{
    unsigned int channel[VIDEO_CHANNEL_SIZE] = {4,2,3,5};

    struct cap_sink_t sink[VIDEO_CHANNEL_SIZE];
    struct cap_src_t source[VIDEO_CHANNEL_SIZE];
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        sink[i].pixfmt = PIX_FMT_UYVY;
        sink[i].width = 704;
        sink[i].height = 574;
        sink[i].size = sink[i].width*sink[i].height*2;
        sink[i].crop_x = 0;
        sink[i].crop_y = 0;
        sink[i].crop_w = 704;
        sink[i].crop_h = 574;

        source[i].width = 704;
        source[i].height = 574;
#if USE_YUV4PANO
        source[i].pixfmt = PIX_FMT_UYVY;
        source[i].size = source[i].width*source[i].height*2;
#else
        source[i].pixfmt = PIX_FMT_BGR24;
        source[i].size = source[i].width*source[i].height*3;
#endif
    }


    Controller controller;
    ICapture* capture = controller.initCaptureModule(channel, VIDEO_CHANNEL_SIZE, sink, source);

#if USE_YUV4PANO
    IPanoImage* panoImage = controller.initPanoImageModule(
                capture,
                704, 574, PIX_FMT_UYVY,
                424, 600, PIX_FMT_UYVY,
                "/home/root/ckt-demo/PanoConfig.bin", true);
#else
    IPanoImage* panoImage = controller.initPanoImageModule(
                capture,
                704, 574, PIX_FMT_BGR24,
                424, 600, PIX_FMT_BGR24,
                "/home/root/ckt-demo/PanoConfig.bin", true);
#endif

    IRender* render = controller.initRenderModule(
                capture,
                NULL,
                panoImage,
                439, 10, 570, 574,
                0, 0, 424, 600);

    controller.startModules(VIDEO_FPS_15);

 //   controller.startLoop(VIDEO_FPS_15);

    while (true)
    {
         sleep(10000);
    }

    return 0;
}
