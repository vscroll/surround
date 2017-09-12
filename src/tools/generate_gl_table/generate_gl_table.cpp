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

static void stitch_gl_init(const std::string configPath,
        cv::Mat& lutHor,
        cv::Mat& lutVer,
    	cv::Mat& mask)
{
#if DEBUG_STITCH
    std::cout << "System Initialization:" << configPath << std::endl;
#endif
	cv::FileStorage fs(configPath, cv::FileStorage::READ);
	fs["Map_1"] >> lutHor;
	fs["Map_2"] >> lutVer;
	fs["Mask"] >> mask;
	fs.release();

    return;
}

static int writeInt(char* filename, char* tablename, int count_per_line, cv::Mat& data)
{
    char header[256] = {0};
    int size = data.cols*data.rows*data.cols;
    sprintf(header, "int %s[%d] = int[](\n", tablename, size);

    FILE* fd = fopen(filename, "at+");
    if (fd < 0)
    {
        printf("faild to open %s\n", filename);
        return -1;
    }

    printf("start write %s\n", filename);
    fwrite(header, 1, strlen(header), fd);
    int count = 0;
    for (int i = 0; i < data.rows; ++i)
    {
        for (int j = 0; j < data.cols; ++j)
        {
            int value = data.ptr<float>(i)[j];
            char byte[32] = {0};
            count++;
            if (count == size)
            {
                sprintf(byte, "%d\n", value);
            }
            else if (count%count_per_line == 0)
            {
                sprintf(byte, "%d,\n", value);
            }
            else
            {
                sprintf(byte, "%d,", value);
            }
            fwrite(byte, 1, strlen(byte), fd);
        }
    }

    char tail[8] = {");\n"};
    fwrite(tail, 1, strlen(tail), fd);
    fclose(fd);
    printf("end write %s\n", filename);
    return 0;
}

static int writeFloat(char* filename, char* tablename, int count_per_line, cv::Mat& data)
{
    char header[256] = {0};
    int size = data.cols*data.rows*data.cols;
    sprintf(header, "const float %s[%d] = float[](\n", tablename, size);

    FILE* fd = fopen(filename, "at+");
    if (fd < 0)
    {
        printf("faild to open %s\n", filename);
        return -1;
    }

    printf("start write %s\n", filename);
    fwrite(header, 1, strlen(header), fd);
    int count = 0;
    for (int i = 0; i < data.rows; ++i)
    {
        for (int j = 0; j < data.cols; ++j)
        {
            float value = data.ptr<float>(i)[j];
            char byte[32] = {0};
            count++;
            if (count == size)
            {
                sprintf(byte, "%0.2f\n", value);
            }
            else if (count%count_per_line == 0)
            {
                sprintf(byte, "%0.2f,\n", value);
            }
            else
            {
                sprintf(byte, "%0.2f,", value);
            }
            fwrite(byte, 1, strlen(byte), fd);
        }
    }

    char tail[8] = {");\n"};
    fwrite(tail, 1, strlen(tail), fd);
    fclose(fd);
    printf("end write %s\n", filename);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("\n usage: generate_gl_table *.xml count_per_line) \n");
        return -1;
    }

    cv::Mat lutHor;
    cv::Mat lutVer;
    cv::Mat mask;
    stitch_gl_init(argv[1], lutHor, lutVer, mask);

    int count_per_line = atoi(argv[2]);
    writeFloat("panorama_rgb_hor.lut", "lutHor", count_per_line, lutHor);
    writeFloat("panorama_rgb_ver.lut", "lutVer", count_per_line, lutVer);
    writeFloat("panorama_rgb_mask.lut", "mask", count_per_line, mask);

    return 0;
}
