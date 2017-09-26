#include "v4l2.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <linux/ipu.h>
#include "common.h"

V4l2::V4l2()
{
}

void V4l2::getVideoCap(int fd)
{
    struct v4l2_capability cap;
    if (-1 != ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        std::cout << "V4l2::getVideoCap"
                 << ", driver:" << cap.driver
                 << ", card:" << cap.card
                 << ", version:" << cap.version
		 << std::endl;
    }
}

void V4l2::getVideoFmt(int fd, unsigned int* pixfmt, unsigned int* width, unsigned int* height)
{
    struct v4l2_fmtdesc fmtDes;
    memset(&fmtDes, 0, sizeof(fmtDes));
    fmtDes.index = 0;
    fmtDes.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtDes) != -1)
    {
        fmtDes.index++;
        std::cout << "V4l2::getVideoFmt"
                 << ", support format:" << fmtDes.index
                 << ", format:" << fmtDes.pixelformat
                 << ", format desciption:" << (char*)(fmtDes.description)
		 << std::endl;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 != ioctl(fd, VIDIOC_G_FMT, &fmt))
    {
        *width = fmt.fmt.pix.width;
        *height = fmt.fmt.pix.height;
        *pixfmt = fmt.fmt.pix.pixelformat;
        std::cout << "V4l2::getVideoFmt"
                 << ", current format:" << fmt.fmt.pix.pixelformat
                 << ", width:" << fmt.fmt.pix.width
                 << ", height:" << fmt.fmt.pix.height
		 << std::endl;
    }

    std::cout << "V4l2::getVideoFmt"
             << ", V4L2_PIX_FMT_RGB555:" << V4L2_PIX_FMT_RGB555
	     << std::endl;
    std::cout << "V4l2::getVideoFmt"
             << ", V4L2_PIX_FMT_RGB565:" << V4L2_PIX_FMT_RGB565
	     << std::endl;
    std::cout << "V4l2::getVideoFmt"
             << ", V4L2_PIX_FMT_YUYV:" << V4L2_PIX_FMT_YUYV
	     << std::endl;
    std::cout << "V4l2::getVideoFmt"
             << ", V4L2_PIX_FMT_UYVY:" << V4L2_PIX_FMT_UYVY
	     << std::endl;
}

int V4l2::setVideoFmt(int fd, unsigned int pixfmt, unsigned int width, unsigned int height)
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int result = ioctl(fd, VIDIOC_G_FMT, &fmt);
    if (-1 == result)
    {
        std::cout << "V4l2::setVideoFmt"
                 << ", setVideoFmt failed"
                 << ", pixfmt:" << fmt.fmt.pix.pixelformat
                 << ", width:" << fmt.fmt.pix.width
                 << ", height:" << fmt.fmt.pix.height
                 << ", errno:" << errno
		         << std::endl;
        return result;
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = pixfmt;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    result = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if (-1 == result)
    {
        std::cout << "V4l2::setVideoFmt"
                 << ", setVideoFmt failed. "
                 << ", pixfmt:" << fmt.fmt.pix.pixelformat
                 << ", width:" << fmt.fmt.pix.width
                 << ", height:" << fmt.fmt.pix.height
                 << ", errno:" << errno
		         << std::endl;
    }

    return result;
}

int V4l2::getFps(int fd)
{
    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int result = ioctl(fd, VIDIOC_G_PARM, &parm);
    if(-1 != result)
    {
        std::cout << "V4l2::getFps"
                 << ", numerator:" << parm.parm.capture.timeperframe.numerator
                 << ", denominator:" << parm.parm.capture.timeperframe.denominator
                 << ", capturemode:" << parm.parm.capture.capturemode
		 << std::endl;
    }

    return result;
}

int V4l2::setFps(int fd, int fps)
{
    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    parm.parm.capture.timeperframe.denominator = fps;
    parm.parm.capture.timeperframe.numerator = 1;
    int result = ioctl(fd, VIDIOC_S_PARM, &parm);
    if(-1 == result)
    {
        std::cout << "V4l2::setFps"
                 << ", setFps failed:" << fps
                 << ", errno:" << errno
		 << std::endl;
    }

    return result;
}

int V4l2::v4l2ReqBuf(int fd, struct buffer* v4l2_buf, unsigned int buf_count, v4l2_memory mem_type, int fd_ipu, unsigned int frame_size)
{
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = buf_count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = mem_type;
    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        std::cout << "V4l2::initV4l2Buf"
                 << ", REQBUFS failed"
		 << std::endl;
        return -1;
    }

    if (req.count < buf_count) {
        std::cout << "V4l2::initV4l2Buf"
                 << ", Insufficient buffer memory on device"
		 << std::endl;
        return -1;
    }

    if (mem_type == V4L2_MEMORY_MMAP)
    {
        for (unsigned int i = 0; i < req.count; ++i)
        {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = mem_type;
            buf.index = i; // 查询序号为mV4l2Buf的缓冲区，得到其起始物理地址和大小
            if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
            {
                std::cout << "V4l2::initV4l2Buf"
                         << ", QUERYBUF failed"
			 << std::endl;
                return -1;
            }

            v4l2_buf[i].length = buf.length;
            v4l2_buf[i].offset = buf.m.offset;
            // 映射内存
            v4l2_buf[i].start = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
            if (MAP_FAILED == v4l2_buf[i].start)
            {
                std::cout << "V4l2::initV4l2Buf"
                        << ", mmap failed"
                        << std::endl;
                return -1;
            }
            memset(v4l2_buf[i].start, 0xFF, v4l2_buf[i].length);
        }
    }
    else if (mem_type == V4L2_MEMORY_USERPTR)
    {
        for (unsigned int i = 0; i < req.count; ++i)
        {
            //unsigned int page_size = getpagesize();
            unsigned int buf_size = frame_size;
           // buf_size = (buf_size + page_size - 1) & ~(page_size - 1);
            v4l2_buf[i].length = v4l2_buf[i].offset = buf_size;
            if (-1 == ioctl(fd_ipu, IPU_ALLOC, &v4l2_buf[i].offset))
            {
                std::cout << "V4l2::initV4l2Buf"
                        << ", IPU_ALLOC failed"
                        << std::endl;
                return -1;
            }

            v4l2_buf[i].start = mmap(0, buf_size, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd_ipu, v4l2_buf[i].offset);
            if (NULL == v4l2_buf[i].start)
            {
                std::cout << "V4l2::initV4l2Buf"
                        << ", ipu map failed"
                        << std::endl;
                return -1;
            }
/*
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = mem_type;
            buf.index = i; // 查询序号为mV4l2Buf的缓冲区，得到其起始物理地址和大小
            buf.length = v4l2_buf[i].length;
            buf.m.userptr = (unsigned long) v4l2_buf[i].start;
            if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
            {
                std::cout << "V4l2::initV4l2Buf"
                        << " QUERYBUF failed";
                return -1;
            }
*/
        }
    }

    return 0;
}

int V4l2::startCapture(int fd, struct buffer* v4l2_buf, unsigned int buf_count, v4l2_memory mem_type)
{
    unsigned int i = 0;
    for (i = 0; i < buf_count; ++i)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = mem_type;
        buf.index = i;
        buf.length = v4l2_buf[i].length;
        if (mem_type == V4L2_MEMORY_MMAP)
        {
            buf.m.offset = v4l2_buf[i].offset;
        }
        else if (mem_type == V4L2_MEMORY_USERPTR)
        {
            buf.m.offset = (unsigned int)v4l2_buf[i].start;
        }

        // 将缓冲帧放入队列
        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            std::cout << "V4l2::startCapture"
                    << ", QBUF failed"
		    << std::endl;
            return -1;
        }
    }

    //开始捕捉图像数据
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
    {
        return -1;
    }

    return 0;
}

void V4l2::stoptCapture(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
}

int V4l2::readFrame(int fd, struct v4l2_buffer* buf, v4l2_memory mem_type)
{
    memset (buf, 0, sizeof(struct v4l2_buffer));
    buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf->memory = mem_type;

    return ioctl(fd, VIDIOC_DQBUF, buf); // 从缓冲区取出一个缓冲帧
}

void V4l2::v4l2QueueBuf(int fd, struct v4l2_buffer* buf)
{
    ioctl(fd, VIDIOC_QBUF, buf); //再将其入列
}

unsigned int V4l2::getPixfmt(unsigned int pixfmt_index)
{
    unsigned int v4l2_pixfmt = PIXFMT_YUYV;
    switch (pixfmt_index)
    {
        case PIXFMT_YUYV:
            v4l2_pixfmt = V4L2_PIX_FMT_YUYV;
            break;
        case PIXFMT_UYVY:
            v4l2_pixfmt = V4L2_PIX_FMT_UYVY;
            break;
        case PIXFMT_RGB555:
            v4l2_pixfmt = V4L2_PIX_FMT_RGB555;
            break;
        case PIXFMT_RGB565:
            v4l2_pixfmt = V4L2_PIX_FMT_RGB565;
            break;
        case PIXFMT_RGB24:
            v4l2_pixfmt = V4L2_PIX_FMT_RGB24;
            break;
        case PIXFMT_BGR24:
            v4l2_pixfmt = V4L2_PIX_FMT_BGR24;
            break;
        case PIXFMT_RGB32:
            v4l2_pixfmt = V4L2_PIX_FMT_RGB32;
            break;
        case PIXFMT_BGR32:
            v4l2_pixfmt = V4L2_PIX_FMT_BGR32;
            break;
        case PIXFMT_YUV422P:
            v4l2_pixfmt = V4L2_PIX_FMT_YUV422P;
            break;
        case PIXFMT_NV12:
            v4l2_pixfmt = V4L2_PIX_FMT_NV12;
            break;
        default:
            break;
    }

    return v4l2_pixfmt;
}

unsigned int V4l2::getIPUPixfmt(unsigned int v4l2_pixfmt)
{
    unsigned int ipu_pixfmt = IPU_PIX_FMT_YUYV;
    switch (v4l2_pixfmt)
    {
        case V4L2_PIX_FMT_YUYV:
            ipu_pixfmt = IPU_PIX_FMT_YUYV;
            break;
        case V4L2_PIX_FMT_UYVY:
            ipu_pixfmt = IPU_PIX_FMT_UYVY;
            break;
        case V4L2_PIX_FMT_RGB555:
            ipu_pixfmt = IPU_PIX_FMT_RGB555;
            break;
        case V4L2_PIX_FMT_RGB565:
            ipu_pixfmt = IPU_PIX_FMT_RGB565;
            break;
        case V4L2_PIX_FMT_RGB24:
            ipu_pixfmt = IPU_PIX_FMT_RGB24;
            break;
        case V4L2_PIX_FMT_BGR24:
            ipu_pixfmt = IPU_PIX_FMT_BGR24;
            break;
        case V4L2_PIX_FMT_RGB32:
            ipu_pixfmt = IPU_PIX_FMT_RGB32;
            break;
        case V4L2_PIX_FMT_BGR32:
            ipu_pixfmt = IPU_PIX_FMT_BGR32;
            break;
        case V4L2_PIX_FMT_YUV422P:
            ipu_pixfmt = IPU_PIX_FMT_YUV420P;
            break;
        case V4L2_PIX_FMT_NV12:
            ipu_pixfmt = IPU_PIX_FMT_NV12;
            break;
        default:
            break;
    }

    return ipu_pixfmt;
}

unsigned int V4l2::getVideoSize(unsigned int v4l2_pixfmt, unsigned int width, unsigned int height)
{
    unsigned int bpp;

    switch (v4l2_pixfmt)
    {
        case V4L2_PIX_FMT_RGB565:
            /*interleaved 422*/
        case V4L2_PIX_FMT_YUYV:
        case V4L2_PIX_FMT_UYVY:
            /*non-interleaved 422*/
        case V4L2_PIX_FMT_YUV422P:
            bpp = 16;
            break;
        case V4L2_PIX_FMT_BGR24:
        case V4L2_PIX_FMT_RGB24:
            bpp = 24;
            break;
        case V4L2_PIX_FMT_BGR32:
        case V4L2_PIX_FMT_RGB32:
            bpp = 32;
            break;
            /*non-interleaved 420*/
        case V4L2_PIX_FMT_NV12:
            bpp = 12;
            break;
        default:
            bpp = 8;
            break;
    }

    return width*height*bpp/8;
}

