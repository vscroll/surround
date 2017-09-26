#include "imxipu.h"
#include <linux/ipu.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include "common.h"

IMXIPU::IMXIPU()
{
}

int IMXIPU::allocIPUBuf(int fd, struct buffer* ipu_buf,  unsigned int frame_size)
{
    //unsigned int page_size = getpagesize();
    unsigned int buf_size = frame_size;
    // buf_size = (buf_size + page_size - 1) & ~(page_size - 1);
    ipu_buf->length = ipu_buf->offset = buf_size;
    if (-1 == ioctl(fd, IPU_ALLOC, &ipu_buf->offset))
    {
        std::cout << "IMXIPU::initV4l2Buf"
                 << ", IPU_ALLOC failed"
		 << std::endl;
        return -1;
    }

    ipu_buf->start = mmap(0, buf_size, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, ipu_buf->offset);
    if (NULL == ipu_buf->start)
    {
        std::cout << "IMXIPU::initV4l2Buf"
                 << ", ipu map failed"
		 << std::endl;
        return -1;
    }

    return 0;
}

int IMXIPU::freeIPUBuf(int fd, struct buffer* ipu_buf)
{
    return -1;
}

int IMXIPU::IPUTask(int fd, struct ipu_task* task)
{
    return ioctl(fd, IPU_QUEUE_TASK, task);
}

unsigned int IMXIPU::getPixfmt(unsigned int pixfmt_index)
{
    unsigned int ipu_pixfmt = IPU_PIX_FMT_YUYV;
    switch (pixfmt_index)
    {
        case PIXFMT_YUYV:
            ipu_pixfmt = IPU_PIX_FMT_YUYV;
            break;
        case PIXFMT_UYVY:
            ipu_pixfmt = IPU_PIX_FMT_UYVY;
            break;
        case PIXFMT_RGB555:
            ipu_pixfmt = IPU_PIX_FMT_RGB555;
            break;
        case PIXFMT_RGB565:
            ipu_pixfmt = IPU_PIX_FMT_RGB565;
            break;
        case PIXFMT_RGB24:
            ipu_pixfmt = IPU_PIX_FMT_RGB24;
            break;
        case PIXFMT_BGR24:
            ipu_pixfmt = IPU_PIX_FMT_BGR24;
            break;
        case PIXFMT_RGB32:
            ipu_pixfmt = IPU_PIX_FMT_RGB32;
            break;
        case PIXFMT_BGR32:
            ipu_pixfmt = IPU_PIX_FMT_BGR32;
            break;
        case PIXFMT_YUV422P:
            ipu_pixfmt = IPU_PIX_FMT_YUV422P;
            break;
        case PIXFMT_NV12:
            ipu_pixfmt = IPU_PIX_FMT_NV12;
            break;
        default:
            break;
    }
    
    return ipu_pixfmt;
}

unsigned int IMXIPU::getVideoSize(unsigned int ipu_pixfmt, unsigned int width, unsigned int height)
{
    unsigned int bpp;

    switch (ipu_pixfmt)
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

    return width*height*bpp/8;
}
