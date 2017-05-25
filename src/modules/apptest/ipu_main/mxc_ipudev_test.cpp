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
    int fd, fd_fb, isize, ovsize, alpsize, cnt = 50;
    int blank, ret;
    FILE * file_in = NULL;
    struct ipu_task task;
    struct fb_var_screeninfo fb_var;
    struct fb_fix_screeninfo fb_fix;
    void *inbuf, *ovbuf, *alpbuf, *vdibuf;
    fd = open("/dev/mxc_ipu", O_RDWR, 0);
    fd_fb = open("/dev/fb1", O_RDWR, 0);
    file_in = fopen(argv[argc-1], "rb");
    memset(&task, 0, sizeof(task));
    /* input setting */
    task.input.width = 320;
    task.input.height = 240;
    task.input.crop.pos.x = 0;
    task.input.crop.pos.y = 0;
    task.input.crop.w = 0;
    task.input.crop.h = 0;
    task.input.format = IPU_PIX_FMT_YUV420P;
    isize = task.input.paddr =
        task.input.width * task.input.height
        * fmt_to_bpp(task.input.format)/8;

    ioctl(fd, IPU_ALLOC, &task.input.paddr);
    inbuf = mmap(0, isize, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, task.input.paddr);
    /*overlay setting */
    task.overlay_en = 1;
    task.overlay.width = 1024;
    task.overlay.height = 600;
    task.overlay.crop.pos.x = 0;
    task.overlay.crop.pos.y = 0;
    task.overlay.crop.w = 0;
    task.overlay.crop.h = 0;
    task.overlay.format = IPU_PIX_FMT_RGB24;
#ifdef GLOBAL_ALP
    task.overlay.alpha.mode = IPU_ALPHA_MODE_GLOBAL;
    task.overlay.alpha.gvalue = 255;
    task.overlay.colorkey.enable = 1;
    task.overlay.colorkey.value = 0x555555;
#else
    task.overlay.alpha.mode = IPU_ALPHA_MODE_LOCAL;
    alpsize = task.overlay.alpha.loc_alp_paddr =
        task.overlay.width * task.overlay.height;
    ioctl(fd, IPU_ALLOC, &task.overlay.alpha.loc_alp_paddr);
    alpbuf = mmap(0, alpsize, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, task.overlay.alpha.loc_alp_paddr);
    memset(alpbuf, 0x00, alpsize/4);
    memset(alpbuf+alpsize/4, 0x55, alpsize/4);
    memset(alpbuf+alpsize/2, 0x80, alpsize/4);
    memset(alpbuf+alpsize*3/4, 0xff, alpsize/4);
#endif
    ovsize = task.overlay.paddr =
        task.overlay.width * task.overlay.height
        * fmt_to_bpp(task.overlay.format)/8;
    ioctl(fd, IPU_ALLOC, &task.overlay.paddr);
    ovbuf = mmap(0, ovsize, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, task.overlay.paddr);

#ifdef GLOBAL_ALP
    memset(ovbuf, 0x55, ovsize/4);
    memset(ovbuf+ovsize/4, 0xff, ovsize/4);
    memset(ovbuf+ovsize/2, 0x55, ovsize/4);
    memset(ovbuf+ovsize*3/4, 0x00, ovsize/4);
#else
    memset(ovbuf, 0x55, ovsize);
#endif

    /* output setting*/
    task.output.width = 1024;
    task.output.height = 600;
    task.output.crop.pos.x = 0;
    task.output.crop.pos.y = 0;
    task.output.crop.w = 0;
    task.output.crop.h = 0;
    task.output.format = IPU_PIX_FMT_RGB565;
    task.output.rotate = IPU_ROTATE_NONE;
    ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var);
    fb_var.xres = task.output.width;
    fb_var.xres_virtual = fb_var.xres;
    fb_var.yres = task.output.height;
    fb_var.yres_virtual = fb_var.yres * 3;
    fb_var.activate |= FB_ACTIVATE_FORCE;
    fb_var.nonstd = task.output.format;
    fb_var.bits_per_pixel = fmt_to_bpp(task.output.format);
    ioctl(fd_fb, FBIOPUT_VSCREENINFO, &fb_var);
    ioctl(fd_fb, FBIOGET_VSCREENINFO, &fb_var);
    ioctl(fd_fb, FBIOGET_FSCREENINFO, &fb_fix);
    task.output.paddr = fb_fix.smem_start;
    blank = FB_BLANK_UNBLANK;

    ioctl(fd_fb, FBIOBLANK, blank);
    task.priority = IPU_TASK_PRIORITY_NORMAL;
    task.task_id = IPU_TASK_ID_ANY;
    task.timeout = 1000;
again:
    ret = ioctl(fd, IPU_CHECK_TASK, &task);
    if (ret != IPU_CHECK_OK) {
        if (ret > IPU_CHECK_ERR_MIN) {
            if (ret == IPU_CHECK_ERR_SPLIT_INPUTW_OVER) {
                task.input.crop.w -= 8;
                goto again;
            }
            if (ret == IPU_CHECK_ERR_SPLIT_INPUTH_OVER) {
                task.input.crop.h -= 8;
                goto again;
            }
            if (ret == IPU_CHECK_ERR_SPLIT_OUTPUTW_OVER) {
                task.output.crop.w -= 8;
                goto again;
            }
            if (ret == IPU_CHECK_ERR_SPLIT_OUTPUTH_OVER) {
                task.output.crop.h -= 8;
                goto again;
            }
            ret = -1;
            return ret;
        }
    }

    dump_ipu_task(&task);
    while (--cnt > 0) {
        fread(inbuf, 1, isize, file_in);
        ioctl(fd, IPU_QUEUE_TASK, &task);
    }
    munmap(ovbuf, ovsize);
    ioctl(fd, IPU_FREE, task.input.paddr);
    ioctl(fd, IPU_FREE, task.overlay.paddr);

    close(fd);
    close(fd_fb);
    fclose(file_in);
}
