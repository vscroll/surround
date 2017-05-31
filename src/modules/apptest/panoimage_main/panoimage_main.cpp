// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "IPanoImage.h"
#include "panoimageimpl.h"

int main (int argc, char **argv)
{
    IPanoImage* panoImage = new PanoImageImpl();
    panoImage->init(NULL,
            CAPTURE_VIDEO_RES_X, CAPTURE_VIDEO_RES_Y, V4L2_PIX_FMT_BGR24,
            RENDER_VIDEO_RES_PANO_X, RENDER_VIDEO_RES_PANO_Y, V4L2_PIX_FMT_BGR24,
            "/home/root/ckt-demo/PanoConfig.bin", true);
    panoImage->start(VIDEO_FPS_15);

    while (true)
    {
        sleep(100);
    }

    return 0;
}
