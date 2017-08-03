#include "controllercl.h"
#include <iostream>
#include "util.h"
#include "IConfig.h"
#include "ICapture.h"
#include "IPanoImage.h"
#include "panoimageimpl.h"
#include "panosourceshmwriteworker.h"

ControllerCL::ControllerCL()
{
    mPanoImage = NULL;
    mPanoSourceSHMWriteWorker = NULL;
}

ControllerCL::~ControllerCL()
{
}

int ControllerCL::startPanoImageModule()
{
    if (NULL == mConfig
        || NULL == mCapture)
    {
        return -1;
    }

    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    char algoCfgPathName[1024] = {0};
    sprintf(algoCfgPathName, "%sFishToPanoYUV.xml", procPath);

    int accelPolicy = mConfig->getAccelPolicy();

    int stitchFPS = mConfig->getStitchFPS();
    if (stitchFPS <= 0)
    {
        stitchFPS = VIDEO_FPS_15;
    }

    mPanoImage = new PanoImageImpl();
    mPanoImage->init(mCapture,
            CAPTURE_VIDEO_RES_X, CAPTURE_VIDEO_RES_Y, V4L2_PIX_FMT_UYVY,
            RENDER_VIDEO_RES_PANO_X, RENDER_VIDEO_RES_PANO_Y, V4L2_PIX_FMT_UYVY,
            algoCfgPathName, accelPolicy);
    mPanoImage->start(stitchFPS);

    if (accelPolicy != ACCEL_POLICY_OPENCL_RENDER)
    {
        mPanoSourceSHMWriteWorker = new PanoSourceSHMWriteWorker(mPanoImage);
        mPanoSourceSHMWriteWorker->start(stitchFPS);
    }

    return 0;
}

void ControllerCL::stopPanoImageModule()
{
    if (NULL != mPanoSourceSHMWriteWorker)
    {
        mPanoSourceSHMWriteWorker->stop();
        delete mPanoSourceSHMWriteWorker;
        mPanoSourceSHMWriteWorker = NULL;
    }

    if (NULL != mPanoImage)
    {
        mPanoImage->stop();
        delete mPanoImage;
        mPanoImage = NULL;
    }  
}
