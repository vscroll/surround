#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "common.h"
#include "IRender.h"
#include "renderimpl.h"
#include "IConfig.h"
#include "configimpl.h"
#include "util.h"

int main (int argc, char **argv)
{
    IConfig* config = new ConfigImpl();
    char cfgPath[1024] = {0};
    if (Util::getAbsolutePath(cfgPath, 1024) < 0)
    {
        return -1;
    }

    char cfgPathName[1024] = {0};
    sprintf(cfgPathName, "%sconfig.ini", cfgPath);
    if (config->loadFromFile(cfgPathName) < 0)
    {
        return -1;
    }

    int sideLeft;
    int sideTop;
    int sideWidth;
    int sideHeight;
    if (config->getSideRect(&sideLeft, &sideTop, &sideWidth, &sideHeight) < 0)
    {
        return -1;
    }
    std::cout << "render_main side"
            << ", left:" << sideLeft
            << ", top:" << sideTop
            << ", width:" << sideWidth
            << ", height:" << sideHeight
            << std::endl;

    int sideCropLeft;
    int sideCropTop;
    int sideCropWidth;
    int sideCropHeight;
    if (config->getSideCrop(&sideCropLeft, &sideCropTop, &sideCropWidth, &sideCropHeight) < 0)
    {
        return -1;
    }
    std::cout << "render_main side crop"
            << ", left:" << sideCropLeft
            << ", top:" << sideCropTop
            << ", width:" << sideCropWidth
            << ", height:" << sideCropHeight
            << std::endl;

    int markLeft;
    int markTop;
    int markWidth;
    int markHeight;
    if (config->getMarkRect(&markLeft, &markTop, &markWidth, &markHeight) < 0)
    {
        return -1;
    }
    std::cout << "render_main mark"
            << ", left:" << markLeft
            << ", top:" << markTop
            << ", width:" << markWidth
            << ", height:" << markHeight
            << std::endl;

    int panoLeft;
    int panoTop;
    int panoWidth;
    int panoHeight;
    if (config->getPanoRect(&panoLeft, &panoTop, &panoWidth, &panoHeight) < 0)
    {
        return -1;
    }
    std::cout << "render_main pano"
            << ", left:" << panoLeft
            << ", top:" << panoTop
            << ", width:" << panoWidth
            << ", height:" << panoHeight
            << std::endl;

    int sideFPS = config->getSideFPS();
    if (sideFPS <= 0)
    {
        sideFPS = VIDEO_FPS_15;
    }

    int markFPS = config->getMarkFPS();
    if (markFPS <= 0)
    {
        markFPS = VIDEO_FPS_15;
    }

    int panoFPS = config->getPanoFPS();
    if (panoFPS <= 0)
    {
        panoFPS = VIDEO_FPS_15;
    }

    RenderImpl* render = new RenderImpl();
    if (sideLeft >= 0
        && sideTop >= 0
        && sideWidth > 0
        && sideHeight > 0)
    {
        render->setCaptureModule(NULL);
	    render->setSideImageCrop(sideCropLeft, sideCropTop, sideCropWidth, sideCropHeight);
        render->setSideImageRect(sideLeft, sideTop, sideWidth, sideHeight);
        render->startRenderSide(sideFPS);
    }

    if (markLeft >= 0
        && markTop >= 0
        && markWidth > 0
        && markHeight > 0)
    {
        render->setMarkRect(markLeft, markTop, markWidth, markHeight);
        render->startRenderMark(markFPS);
    }

    if (panoLeft >= 0
        && panoTop >= 0
        && panoWidth > 0
        && panoHeight > 0)
    {
        render->setPanoImageModule(NULL);          
        render->setPanoImageRect(panoLeft, panoTop, panoWidth, panoHeight);
        render->startRenderPano(panoFPS);
    }

    while (true)
    {
         sleep(10000);
    }

    delete config;
    config = NULL;

    return 0;
}
