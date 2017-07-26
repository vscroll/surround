// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "common.h"
#include "IPanoImage.h"
#include "panoimageimpl.h"
#include "wrap_thread.h"
#include "imageshm.h"
#include "util.h"
#include "IConfig.h"
#include "configimpl.h"
#include "panosourceshmwriteworker.h"


int main (int argc, char **argv)
{
    IConfig* config = new ConfigImpl();

    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    char algoCfgPathName[1024] = {0};
    sprintf(algoCfgPathName, "%sFishToPanoYUV.xml", procPath);

    char cfgPathName[1024] = {0};
    sprintf(cfgPathName, "%sconfig.ini", procPath);
    if (config->loadFromFile(cfgPathName) < 0)
    {
        return -1;
    }

    int accelPolicy = config->getAccelPolicy();

    int fps = config->getStitchFPS();
    if (fps <= 0)
    {
        fps = VIDEO_FPS_15;
    }

    IPanoImage* panoImage = new PanoImageImpl();
    panoImage->init(NULL,
            CAPTURE_VIDEO_RES_X, CAPTURE_VIDEO_RES_Y, V4L2_PIX_FMT_UYVY,
            RENDER_VIDEO_RES_PANO_X, RENDER_VIDEO_RES_PANO_Y, V4L2_PIX_FMT_UYVY,
            algoCfgPathName, accelPolicy);
    panoImage->start(fps);

    PanoSourceSHMWriteWorker* panoSourceSHMWriteWorker = new PanoSourceSHMWriteWorker(panoImage);
    panoSourceSHMWriteWorker->start(fps);

    while (true)
    {
        sleep(100);
    }

    panoSourceSHMWriteWorker->stop();
    delete panoSourceSHMWriteWorker;
    panoSourceSHMWriteWorker = NULL;

    panoImage->stop();
    delete panoImage;
    panoImage = NULL;

    delete config;
    config = NULL;

    return 0;
}
