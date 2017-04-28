// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "controller.h"
#include <opencv/cv.h>

int main (int argc, char **argv)
{	
	Controller controller;
	unsigned int channel[VIDEO_CHANNEL_SIZE] = {4,2,3,5};
	controller.init(channel, VIDEO_CHANNEL_SIZE);

    	controller.start(VIDEO_FPS_15,
			424,
			600,
			"/home/root/ckt-demo/PanoConfig.bin",
			true);

	while (true)
	{
		surround_image_t* surroundImage0 = controller.dequeuePano2DImage();
		if (surroundImage0 != NULL)
		{
			cv::Mat* frame0 = (cv::Mat*)(surroundImage0->frame.data);
			if (frame0 != NULL)	        	
			{
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
	        		delete frame1;
			}
		}
		delete surroundImage1;

		usleep(1000/VIDEO_FPS_15);
	}

	return 0;
}
