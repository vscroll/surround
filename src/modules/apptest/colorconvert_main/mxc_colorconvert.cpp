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
#include <linux/mxcfb.h>
#include <linux/ipu.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

static unsigned int fmt_to_bpp(unsigned int pixelformat)
{
    unsigned int bpp;

    switch (pixelformat)
    {
        case IPU_PIX_FMT_RGB565:
            /*interleaved 422*/
        case IPU_PIX_FMT_YUYV:
        case IPU_PIX_FMT_UYVY:
            /*non-interleaved 422*/
        case IPU_PIX_FMT_YUV422P:
        case IPU_PIX_FMT_YVU422P:
            bpp = 16;
            break;
        case IPU_PIX_FMT_BGR24:
        case IPU_PIX_FMT_RGB24:
        case IPU_PIX_FMT_YUV444:
        case IPU_PIX_FMT_YUV444P:
            bpp = 24;
            break;
        case IPU_PIX_FMT_BGR32:
        case IPU_PIX_FMT_BGRA32:
        case IPU_PIX_FMT_RGB32:
        case IPU_PIX_FMT_RGBA32:
        case IPU_PIX_FMT_ABGR32:
            bpp = 32;
            break;
            /*non-interleaved 420*/
        case IPU_PIX_FMT_YUV420P:
        case IPU_PIX_FMT_YVU420P:
        case IPU_PIX_FMT_YUV420P2:
        case IPU_PIX_FMT_NV12:
        case IPU_PIX_FMT_TILED_NV12:
            bpp = 12;
            break;
        default:
            bpp = 8;
            break;
    }
    return bpp;
}

static void dump_ipu_task(struct ipu_task *t)
{
    printf("====== ipu task ======\n");
    printf("input:\n");
    printf("\tforamt: 0x%x\n", t->input.format);
    printf("\twidth: %d\n", t->input.width);
    printf("\theight: %d\n", t->input.height);
    printf("\tcrop.w = %d\n", t->input.crop.w);
    printf("\tcrop.h = %d\n", t->input.crop.h);
    printf("\tcrop.pos.x = %d\n", t->input.crop.pos.x);
    printf("\tcrop.pos.y = %d\n", t->input.crop.pos.y);
    if (t->input.deinterlace.enable) {
        printf("deinterlace enabled with:\n");
        if (t->input.deinterlace.motion != HIGH_MOTION)
            printf("\tlow/medium motion\n");
        else
            printf("\thigh motion\n");
    }
    printf("output:\n");
    printf("\tforamt: 0x%x\n", t->output.format);
    printf("\twidth: %d\n", t->output.width);
    printf("\theight: %d\n", t->output.height);
    printf("\troate: %d\n", t->output.rotate);
    printf("\tcrop.w = %d\n", t->output.crop.w);
    printf("\tcrop.h = %d\n", t->output.crop.h);
    printf("\tcrop.pos.x = %d\n", t->output.crop.pos.x);
    printf("\tcrop.pos.y = %d\n", t->output.crop.pos.y);
    if (t->overlay_en) {
        printf("overlay:\n");
        printf("\tforamt: 0x%x\n", t->overlay.format);
        printf("\twidth: %d\n", t->overlay.width);
        printf("\theight: %d\n", t->overlay.height);
        printf("\tcrop.w = %d\n", t->overlay.crop.w);
        printf("\tcrop.h = %d\n", t->overlay.crop.h);
        printf("\tcrop.pos.x = %d\n", t->overlay.crop.pos.x);
        printf("\tcrop.pos.y = %d\n", t->overlay.crop.pos.y);
        if (t->overlay.alpha.mode == IPU_ALPHA_MODE_LOCAL)
            printf("combine with local alpha\n");
        else
            printf("combine with global alpha %d\n", t->overlay.alpha.gvalue);
        if (t->overlay.colorkey.enable)
            printf("colorkey enabled with 0x%x\n", t->overlay.colorkey.value);
    }
}

int main(int argc, char *argv[])
{
    int src_fmt = -1;
    int dst_fmt = -1;
    int in_width = 0;
    int in_height = 0;
    char* in_image = NULL;

    if (argc == 5)
    {
        if (strcmp("uyvy2rgb", argv[1]) == 0)
        {
            src_fmt = IPU_PIX_FMT_UYVY;
            dst_fmt = IPU_PIX_FMT_BGR24;
            in_width = atoi(argv[3]);
            in_height = atoi(argv[4]);
        }
        else if (strcmp("yuyv2rgb", argv[1]) == 0)
        {
            src_fmt = IPU_PIX_FMT_YUYV;
            dst_fmt = IPU_PIX_FMT_BGR24;
        }
        else if (strcmp("rgb2uyvy", argv[1]) == 0)
        {
            src_fmt = IPU_PIX_FMT_BGR24;
            dst_fmt = IPU_PIX_FMT_UYVY;
        }
        else if (strcmp("rgb2yuyv", argv[1]) == 0)
        {
            src_fmt = IPU_PIX_FMT_BGR24;
            dst_fmt = IPU_PIX_FMT_YUYV;
        }
        else
        {
        }

        in_width = atoi(argv[3]);
        in_height = atoi(argv[4]);
    }
    
    if (src_fmt == -1
        || dst_fmt == -1)
    {
        printf("\n usage: colorconvert [uyvy2rgb|yuyv2rgb|rgb2uyvy|rgb2yuyv] input in_width in_height) \n");
        return -1;
    }

    char* input = argv[2];
    char* tmp = strrchr(input, '.');
    char outname[256] = {0};
    memcpy(outname, input, strlen(input)-strlen(tmp));
    char output[256] = {0};
    if (dst_fmt == IPU_PIX_FMT_UYVY)
    {
        sprintf(output, "%s_%dx%d.uyvy", outname, in_width, in_height);
    }
    else if (dst_fmt == IPU_PIX_FMT_YUYV)
    {
        sprintf(output, "%s_%dx%d.yuyv", outname, in_width, in_height);
    }
    else if (dst_fmt == IPU_PIX_FMT_BGR24)
    {
        sprintf(output, "%s.jpg", outname, in_width, in_height);
    }
    else
    {
        printf("filename error\n");
        return -1;
    }  

    printf("output filename:%s\n", output);

    if (src_fmt == IPU_PIX_FMT_BGR24)
    {
        IplImage* inputImage = cvLoadImage(input);
        if (NULL != inputImage)
        {
            in_width = inputImage->width;
            in_height = inputImage->height;
            in_image = inputImage->imageData;

            printf("%s: width:%d height:%d channel:%d depth:%d widthstep:%d) \n", 
                input, in_width, in_height, inputImage->nChannels, inputImage->depth, inputImage->widthStep);
        }
    }


    int ipu_fd = open("/dev/mxc_ipu", O_RDWR, 0);
    if (ipu_fd < 0)
    {
        printf("faild to open mxc_ipu\n");
        return -1;
    }

    struct ipu_task task;
    memset(&task, 0, sizeof(task));
    /* input setting */
    task.input.width = in_width;
    task.input.height = in_height;
    task.input.crop.pos.x = 0;
    task.input.crop.pos.y = 0;
    task.input.crop.w = 0;
    task.input.crop.h = 0;
    task.input.format = src_fmt;
    int isize = task.input.paddr =
        task.input.width * task.input.height
        * fmt_to_bpp(task.input.format)/8;

    ioctl(ipu_fd, IPU_ALLOC, &task.input.paddr);
    void* inbuf = mmap(0, isize, PROT_READ | PROT_WRITE,
            MAP_SHARED, ipu_fd, task.input.paddr);
    if (inbuf == NULL)
    {
        printf("faild to IPU_ALLOC inbuf \n");
        return -1;
    }

    if (src_fmt == IPU_PIX_FMT_UYVY
        || src_fmt == IPU_PIX_FMT_YUYV)
    {
        FILE * input_fd = fopen(input, "rb");
        if (input_fd < 0)
        {
            printf("faild to open %s\n", input);
            return -1;
        }

        fread(inbuf, 1, isize, input_fd);
        fclose(input_fd);
    }
    else if (src_fmt == IPU_PIX_FMT_BGR24)
    {
        memcpy(inbuf, in_image, isize);
    }

    /* output setting*/
    task.output.width = in_width;
    task.output.height = in_height;
    task.output.crop.pos.x = 0;
    task.output.crop.pos.y = 0;
    task.output.crop.w = 0;
    task.output.crop.h = 0;
    task.output.format = dst_fmt;
    task.output.rotate = IPU_ROTATE_NONE;

    int osize = task.output.paddr =
        task.output.width * task.output.height
        * fmt_to_bpp(task.output.format)/8;

    ioctl(ipu_fd, IPU_ALLOC, &task.output.paddr);
    void* outbuf = mmap(0, osize, PROT_READ | PROT_WRITE,
            MAP_SHARED, ipu_fd, task.output.paddr);
    if (outbuf == NULL)
    {
        printf("faild to IPU_ALLOC outbuf \n");
        return -1;
    }

    task.priority = IPU_TASK_PRIORITY_NORMAL;
    task.task_id = IPU_TASK_ID_ANY;
    task.timeout = 1000;

    int ret = ioctl(ipu_fd, IPU_CHECK_TASK, &task);
    if (ret != IPU_CHECK_OK) {
        printf("faild to IPU_CHECK_TASK \n");
        return -1;
    }

    dump_ipu_task(&task);
    if (ioctl(ipu_fd, IPU_QUEUE_TASK, &task) < 0)
    {
        printf("faild to IPU_QUEUE_TASK \n");
        return -1;
    }  

    if (dst_fmt == IPU_PIX_FMT_UYVY
        || dst_fmt == IPU_PIX_FMT_YUYV)
    {
        printf("output: %s \n", output);
        FILE * output_fd = fopen(output, "wb");
        if (output_fd < 0)
        {
            printf("faild to open %s\n", output);
            return -1;
        }
        fwrite(outbuf, 1, osize, output_fd);
        fclose(output_fd);
    }
    else if (dst_fmt == IPU_PIX_FMT_BGR24)
    {
        IplImage *pIplImage = cvCreateImage(cvSize(in_width, in_height), IPL_DEPTH_8U, 3);
        if (NULL != pIplImage)
        {
            memcpy(pIplImage->imageData, outbuf, osize);

            cvSaveImage(output, pIplImage, 0);

            cvReleaseImage(&pIplImage);
        }
    }
    else
    {
    }

    munmap(inbuf, isize);
    munmap(outbuf, osize);
    ioctl(ipu_fd, IPU_FREE, task.input.paddr);
    ioctl(ipu_fd, IPU_FREE, task.output.paddr);

    close(ipu_fd);
}
