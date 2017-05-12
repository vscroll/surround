// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "controller.h"
#include "imageshm.h"
#include <opencv/cv.h>

int main (int argc, char **argv)
{
    int ret = -1;
    
    unsigned int channel[VIDEO_CHANNEL_SIZE] = {4,2,3,5};

    struct cap_sink_t sink[VIDEO_CHANNEL_SIZE];
    struct cap_src_t source[VIDEO_CHANNEL_SIZE];
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        sink[i].pixfmt = IN_PIX_FMT_UYVY;
        sink[i].width = 704;
        sink[i].height = 574;
        sink[i].crop_x = 0;
        sink[i].crop_y = 0;
        sink[i].crop_w = 704;
        sink[i].crop_h = 574;

        source[i].pixfmt = OUT_PIX_FMT_BGR24;
        source[i].width = 704;
        source[i].height = 574;
    }

    Controller controller;
    controller.init(channel, VIDEO_CHANNEL_SIZE, sink, source);

    controller.start(VIDEO_FPS_15,
            0, 0, 424, 600,
	    0, 0, 0, 0,
            "/home/root/ckt-demo/PanoConfig.bin",
            true); 

    ImageSHM sideSHM;
    ret = sideSHM.create((key_t)SHM_SIDE_ID, SHM_SIDE_SIZE);
    if (ret  < 0)
    {
	return -1;
    }

    ImageSHM panoSHM;
    ret = panoSHM.create((key_t)SHM_PANO2D_ID, SHM_PANO2D_SIZE);
    if (ret  < 0)
    {
	return -1;
    } 

    while (true)
    {
        surround_image_t* surroundImage0 = controller.dequeuePano2DImage();
        if (surroundImage0 != NULL)
        {
            cv::Mat* frame0 = (cv::Mat*)(surroundImage0->frame.data);
            if (frame0 != NULL)	        	
            {
		struct image_shm_header_t header = {};
		header.width = surroundImage0->frame.width;
		header.height = surroundImage0->frame.height;
		header.pixfmt = surroundImage0->frame.pixfmt;
		header.timestamp = surroundImage0->timestamp;
		double start = clock();
		ret = panoSHM.writeImage(&header, (unsigned char*)frame0->data, header.width*header.height*3);
	        if (ret < 0)
	        {
                    std::cout << "shm write err" << std::endl;
                }
		std::cout << "============ shm write time: " << (clock()-start)/CLOCKS_PER_SEC
			  << " width:" << header.width
                          << " height:" << header.height
                          << " timestamp:" << header.timestamp
			  << std::endl;

                delete frame0;
            }
            delete surroundImage0;
        }

        surround_image_t* surroundImage1 = controller.dequeueSideImage(0);
        if (surroundImage1 != NULL)
        {
            cv::Mat* frame1 = (cv::Mat*)(surroundImage1->frame.data);
            if (frame1 != NULL)	        	
            {
		struct image_shm_header_t header = {};
		header.width = surroundImage1->frame.width;
		header.height = surroundImage1->frame.height;
		header.pixfmt = surroundImage1->frame.pixfmt;
		header.timestamp = surroundImage1->timestamp;
		double start = clock();
		ret = sideSHM.writeImage(&header, (unsigned char*)frame1->data, header.width*header.height*3);
	        if (ret < 0)
	        {
                    std::cout << "shm write err" << std::endl;
                }
		std::cout << "============ shm write time: " << (clock()-start)/CLOCKS_PER_SEC
			  << " width:" << header.width
                          << " height:" << header.height
                          << " timestamp:" << header.timestamp
                          << std::endl;

                delete frame1;
            }
        }
        delete surroundImage1;

        usleep(1000/VIDEO_FPS_15);
    }

    return 0;
}
