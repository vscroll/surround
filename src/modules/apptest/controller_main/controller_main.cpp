// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "controller.h"

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
        sink[i].crop_x = 0;
        sink[i].crop_y = 0;
        sink[i].crop_w = 704;
        sink[i].crop_h = 574;

        source[i].pixfmt = PIX_FMT_BGR24;
        source[i].width = 704;
        source[i].height = 574;
    }

    Controller controller;
    controller.startLoop(VIDEO_FPS_15);

    controller.initCaptureModule(channel, VIDEO_CHANNEL_SIZE, sink, source);
    controller.initPanoImageModule(704, 574, PIX_FMT_BGR24,
                424, 600, PIX_FMT_BGR24,
                "/home/root/ckt-demo/PanoConfig.bin", true);
    controller.startModules(VIDEO_FPS_15);

    while (true)
    {
         usleep(1000);
    }

    return 0;
}
