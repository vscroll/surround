// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "ICapture.h"
#include "captureimpl.h"

int main (int argc, char **argv)
{
    ICapture* capture = new CaptureImpl();
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

        source[i].pixfmt = PIX_FMT_UYVY;
        source[i].width = 704;
        source[i].height = 574;
        source[i].size = source[i].width*source[i].height*2;
    }

    capture->setCapCapacity(sink, source, VIDEO_CHANNEL_SIZE);
    struct cap_src_t focusSource;
    focusSource.pixfmt = sink[0].pixfmt;
    focusSource.width = sink[0].width;
    focusSource.height = sink[0].height;
    focusSource.size = sink[0].size;
    capture->setFocusSource(0, &focusSource);
    capture->openDevice(channel, VIDEO_CHANNEL_SIZE);
    capture->start(VIDEO_FPS_15);

    while (true)
    {
         sleep(10000);
    }

    capture->closeDevice();
    capture->stop();

    return 0;
}
