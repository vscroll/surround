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

class PanoSourceSHMWriteWorker : public WrapThread
{
public:
    PanoSourceSHMWriteWorker(IPanoImage* panoImage);
    virtual ~PanoSourceSHMWriteWorker();

public:
    virtual void run();

private:
    IPanoImage* mPanoImage;
    ImageSHM* mImageSHM;
};

PanoSourceSHMWriteWorker::PanoSourceSHMWriteWorker(IPanoImage* panoImage)
{
    mPanoImage = panoImage;
    mImageSHM = new ImageSHM();
    mImageSHM->create((key_t)SHM_PANO_SOURCE_ID, SHM_PANO_SOURCE_SIZE);
}

PanoSourceSHMWriteWorker::~PanoSourceSHMWriteWorker()
{
    if (NULL != mImageSHM)
    {
        mImageSHM->destroy();
        delete mImageSHM;
        mImageSHM = NULL;
    }
}

void PanoSourceSHMWriteWorker::run()
{
    if (NULL == mPanoImage
        || NULL == mImageSHM)
    {
        return;
    }

    surround_image_t* source = mPanoImage->dequeuePanoImage();
    if (NULL != source)
    {
#if 0
        clock_t start = clock();
#endif
        mImageSHM->writePanoSources(source);
#if 0
        std::cout << "PanoSourceSHMWriteWorker run: " << (double)(clock()-start)/CLOCKS_PER_SEC
                << " width:" << source->info.width
                << " height:" << source->info.height
                << " size:" << source->info.size
                << " timestamp:" << source->timestamp
                << std::endl;
#endif
        if (NULL != source->data)
        {
            delete (unsigned char*)source->data;
        }

        delete source;
        source = NULL;
    }
}

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
    if (config->loadFile(cfgPathName) < 0)
    {
        return -1;
    }

    bool enableOpenCL = config->enableOpenCL();

    int fps = config->getStitchFPS();
    if (fps <= 0)
    {
        fps = VIDEO_FPS_15;
    }

    IPanoImage* panoImage = new PanoImageImpl();
    panoImage->init(NULL,
            CAPTURE_VIDEO_RES_X, CAPTURE_VIDEO_RES_Y, V4L2_PIX_FMT_UYVY,
            RENDER_VIDEO_RES_PANO_X, RENDER_VIDEO_RES_PANO_Y, V4L2_PIX_FMT_UYVY,
            algoCfgPathName, enableOpenCL);
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

    return 0;
}
