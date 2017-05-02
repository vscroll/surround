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

int main (int argc, char **argv)
{
    unsigned int pano2DHeight = 424;
    unsigned int pano2DWidth = 600;

    cv::Mat stitchMap;
    cv::Mat stitchMapX;
    cv::Mat stitchMapY;
    cv::Mat mask;	
    stitching_init("/home/root/ckt-demo/PanoConfig.bin", stitchMap, mask, true);

    std::cout <<"mStitchMap rows:"<< stitchMap.rows  << " cols:" << stitchMap.cols  << " channel:" << stitchMap.channels() << std::endl;
    std::cout <<"mMask rows:"<< mask.rows  << " cols:" << mask.cols  << " channel:" << mask.channels() << std::endl;
    std::cout <<"mPano2DHeight:"<< pano2DHeight  << " mPano2DWidth:" << pano2DWidth  << std::endl;
    int mapX[pano2DHeight][pano2DWidth];
    int mapY[pano2DHeight][pano2DWidth];
    for (unsigned int i = 0; i < pano2DHeight; i++)
    {
        for (unsigned int j = 0; j < pano2DWidth; j++)
        {
            //mapX[i][j] = stitchMap.ptr<Point2f>(i)[j].x;
            //mapY[i][j] = stitchMap.ptr<Point2f>(i)[j].y;
        }
    }

    stitchMapX = cv::Mat(pano2DHeight, pano2DWidth, CV_32SC1, mapX);
    stitchMapY = cv::Mat(pano2DHeight, pano2DWidth, CV_32SC1, mapY);
    std::cout <<"mStitchMapX rows:"<< stitchMapX.rows  << " cols:" << stitchMapX.cols  << " channel:" << stitchMapX.channels() << std::endl;
    std::cout <<"mStitchMapY rows:"<< stitchMapY.rows  << " cols:" << stitchMapY.cols  << " channel:" << stitchMapY.channels() << std::endl;

    surround_images_t surroundImage;
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        surroundImage.frame[i].data = new cv::Mat(704, 574, CV_8UC3);
        surroundImage.frame[i].width = 704;
        surroundImage.frame[i].height = 574;
        surroundImage.frame[i].pixfmt = V4L2_PIX_FMT_BGR24;
    }

    void* outSide = NULL;
    void* outPano2D = NULL;
    while (true)
    {
	double start = clock();
        stitching_cl(surroundImage.frame[VIDEO_CHANNEL_FRONT].data,
                surroundImage.frame[VIDEO_CHANNEL_REAR].data,
                surroundImage.frame[VIDEO_CHANNEL_LEFT].data,
                surroundImage.frame[VIDEO_CHANNEL_RIGHT].data,
                stitchMapX,
                stitchMapY,
                mask,
                &outPano2D,
                pano2DWidth,
                pano2DHeight,
                &outSide,
                surroundImage.frame[0].width,
                surroundImage.frame[0].height,
                0);
	std::cout <<"stitch time:"<< (clock()-start)/CLOCKS_PER_SEC << std::endl;
        usleep(1000/VIDEO_FPS_15);
    }

    return 0;
}
