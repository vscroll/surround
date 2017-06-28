/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 */

/*
 * The code contained herein is licensed under the GNU Lesser General
 * Public License.  You may obtain a copy of the GNU Lesser General
 * Public License Version 2.1 or later at the following locations:
 *
 * http://www.opensource.org/licenses/lgpl-license.html
 * http://www.gnu.org/copyleft/lgpl.html
 */

/*!
 * @file mxc_ipudev_test.c
 *
 * @brief IPU device lib test implementation
 *
 * @ingroup IPU
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <string.h>
#include "common.h"
#include "IConfig.h"
#include "configimpl.h"
#include "util.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("\n usage: update_crop_info CropInformation config.ini new_config.ini) \n");
        return -1;
    }

	cv::Point start[VIDEO_CHANNEL_SIZE];
	int width[VIDEO_CHANNEL_SIZE];
	int height[VIDEO_CHANNEL_SIZE];
	cv::FileStorage fs1(argv[1], cv::FileStorage::READ);
	for (int i = 0; i < VIDEO_CHANNEL_SIZE; i++)
	{
		std::stringstream index1;
		index1 << "start" << i + 1;
		fs1[index1.str()] >> start[i];

		std::stringstream index2;
		index2 << "width" << i + 1;
		fs1[index2.str()] >> width[i];

		std::stringstream index3;
		index3 << "height" << i + 1;
		fs1[index3.str()] >> height[i];

		printf("%d x=%d y=%d widht=%d height=%d \n", i, start[i].x, start[i].y, width[i], height[i]);
	}
	fs1.release();

    IConfig* config = new ConfigImpl();
    if (config->loadFromFile(argv[2]) < 0)
    {
        std::cout << "capture_main:: load config error"
                << std::endl;
        return -1;
    }

	for (int i = 0; i < VIDEO_CHANNEL_SIZE; i++)
	{	
		config->setSinkCropX(i, start[i].x);
		config->setSinkCropY(i, start[i].y);
		config->setSinkCropWidth(i, width[i]);
		config->setSinkCropHeight(i, height[i]);
	}

    config->saveAsFile(argv[3]);

	delete config;
	config = NULL;

	return 0;
}
