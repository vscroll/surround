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
    if (config->loadFile(cfgPathName) < 0)
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
    std::cout << "render_main mark"
            << ", left:" << panoLeft
            << ", top:" << panoTop
            << ", width:" << panoWidth
            << ", height:" << panoHeight
            << std::endl;

    int fps = config->getRenderFPS();
    if (fps <= 0)
    {
        fps = VIDEO_FPS_15;
    }

    RenderImpl* render = new RenderImpl();
    render->setCaptureModule(NULL);
    render->setSideImageRect(sideLeft, sideTop, sideWidth, sideHeight);
    render->setChannelMarkRect(markLeft, markTop, markWidth, markHeight);

    render->setPanoImageModule(NULL);          
    render->setPanoImageRect(panoLeft, panoTop, panoWidth, panoHeight);

    render->start(fps);

    while (true)
    {
         sleep(10000);
    }

    config->unloadFile();
    delete config;
    config = NULL;

    return 0;
}
