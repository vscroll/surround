// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include <opencv/cv.h>
#include <linux/videodev2.h>
#include "stitch_algorithm.h"
#include "thread.h"

#define MUTIL_THREAD 0

class StitchTestWorker : public Thread
{
public:
    StitchTestWorker();
    virtual ~StitchTestWorker();

    void init();
public:
    virtual void run();

private:
    unsigned int mPano2DWidth;
    unsigned int mPano2DHeight;
    cv::Mat mStitchMap;
    cv::Mat mStitchMapX;
    cv::Mat mStitchMapY;
    cv::Mat mMask;

    surround_images_t mSurroundImage;
};

StitchTestWorker::StitchTestWorker()
{

}

StitchTestWorker::~StitchTestWorker()
{

}

void StitchTestWorker::init()
{
    mPano2DHeight = 424;
    mPano2DWidth = 600;
	
    stitching_init("/home/root/ckt-demo/PanoConfig.bin", mStitchMap, mMask, true);

    std::cout <<"mStitchMap rows:"<< mStitchMap.rows  << " cols:" << mStitchMap.cols  << " channel:" << mStitchMap.channels() << std::endl;
    std::cout <<"mMask rows:"<< mMask.rows  << " cols:" << mMask.cols  << " channel:" << mMask.channels() << std::endl;
    std::cout <<"mPano2DHeight:"<< mPano2DHeight  << " mPano2DWidth:" << mPano2DWidth  << std::endl;
    int mapX[mPano2DHeight][mPano2DWidth] = {0};
    int mapY[mPano2DHeight][mPano2DWidth] = {0};
    for (unsigned int i = 0; i < mPano2DHeight; i++)
    {
        for (unsigned int j = 0; j < mPano2DWidth; j++)
        {
            //mapX[i][j] = mStitchMap.ptr<Point2f>(i)[j].x;
            //mapY[i][j] = mStitchMap.ptr<Point2f>(i)[j].y;
        }
    }

    mStitchMapX = cv::Mat(mPano2DHeight, mPano2DWidth, CV_32SC1, mapX);
    mStitchMapY = cv::Mat(mPano2DHeight, mPano2DWidth, CV_32SC1, mapY);
    std::cout <<"mStitchMapX rows:"<< mStitchMapX.rows  << " cols:" << mStitchMapX.cols  << " channel:" << mStitchMapX.channels() << std::endl;
    std::cout <<"mStitchMapY rows:"<< mStitchMapY.rows  << " cols:" << mStitchMapY.cols  << " channel:" << mStitchMapY.channels() << std::endl;

    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mSurroundImage.frame[i].data = new cv::Mat(704, 574, CV_8UC3);
        mSurroundImage.frame[i].width = 704;
        mSurroundImage.frame[i].height = 574;
        mSurroundImage.frame[i].pixfmt = V4L2_PIX_FMT_BGR24;
    }
}

void StitchTestWorker::run()
{
    void* outSide = NULL;
    void* outPano2D = NULL;
    std::cout <<" stitch start" << std::endl;
    double start = clock();
    stitching_cl(mSurroundImage.frame[VIDEO_CHANNEL_FRONT].data,
                mSurroundImage.frame[VIDEO_CHANNEL_REAR].data,
                mSurroundImage.frame[VIDEO_CHANNEL_LEFT].data,
                mSurroundImage.frame[VIDEO_CHANNEL_RIGHT].data,
                mStitchMapX,
                mStitchMapY,
                mMask,
                &outPano2D,
                mPano2DWidth,
                mPano2DHeight,
                &outSide,
                mSurroundImage.frame[0].width,
                mSurroundImage.frame[0].height,
                0);
    std::cout <<"stitch time:"<< (clock()-start)/CLOCKS_PER_SEC << std::endl;
}


class IdleTestWorker : public Thread
{
public:
    IdleTestWorker();
    virtual ~IdleTestWorker();

public:
    virtual void run();

private:
};

IdleTestWorker::IdleTestWorker()
{

}

IdleTestWorker::~IdleTestWorker()
{

}

void IdleTestWorker::run()
{
    while (true)
    {
        for (unsigned int i = 0; i < 0x0000FFFF; ++i)
        {
	    cv::FileStorage fs0("/home/root/ckt-demo/PanoConfig.bin", cv::FileStorage::READ);
	    cv::FileStorage fs1("/home/root/ckt-demo/PanoConfig.bin", cv::FileStorage::READ);
	    cv::FileStorage fs2("/home/root/ckt-demo/PanoConfig.bin", cv::FileStorage::READ);
	    cv::FileStorage fs3("/home/root/ckt-demo/PanoConfig.bin", cv::FileStorage::READ);
        }

        sleep(1000/VIDEO_FPS_15);
    }
}

int main (int argc, char **argv)
{

    StitchTestWorker worker;
    worker.init();
    worker.start(1000/VIDEO_FPS_15);

#if MUTIL_THREAD
    int num = 10;
    IdleTestWorker idleworker[num];
    for (int i = 0; i < num; ++i)
    {
    	idleworker[i].start(1000/VIDEO_FPS_15);
    }
#endif

    while (true)
    {
        for (unsigned int i = 0; i < 0x0000FFFF; ++i)
        {
	    
        }

        usleep(1000/VIDEO_FPS_15);
    }

    return 0;
}
