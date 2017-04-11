#include "v4l2.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <errno.h>
#include <QDebug>
#include <unistd.h>
#include "common.h"

#if USE_IMX_IPU
#include <linux/ipu.h>
#endif

V4l2::V4l2()
{
}

void V4l2::getVideoCap(int fd)
{
    struct v4l2_capability cap;
    if (-1 != ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        qDebug() << "V4l2::getVideoCap "
                 << " driver:" << cap.driver
                 << ", card:" << cap.card
                 << ", version:" << cap.version;
    }
}

void V4l2::getVideoFmt(int fd, int* width, int* height)
{
    struct v4l2_fmtdesc fmtDes;
    memset(&fmtDes, 0, sizeof(fmtDes));
    fmtDes.index = 0;
    fmtDes.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtDes) != -1)
    {
        fmtDes.index++;
        qDebug() << "V4l2::getVideoFmt"
                 << " support format:" << fmtDes.index
                 << " format:" << fmtDes.pixelformat
                 << " format desciption:" << (char*)(fmtDes.description);
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 != ioctl(fd, VIDIOC_G_FMT, &fmt))
    {
        *width = fmt.fmt.pix.width;
        *height = fmt.fmt.pix.height;
        qDebug() << "V4l2::getVideoFmt "
                 << ", current format:" << fmt.fmt.pix.pixelformat
                 << ", width:" << fmt.fmt.pix.width
                 << ", height:" << fmt.fmt.pix.height;
    }

    qDebug() << "V4l2::getVideoFmt"
             << " V4L2_PIX_FMT_RGB555:" << V4L2_PIX_FMT_RGB555;
    qDebug() << "V4l2::getVideoFmt"
             << " V4L2_PIX_FMT_RGB565:" << V4L2_PIX_FMT_RGB565;
    qDebug() << "V4l2::getVideoFmt"
             << " V4L2_PIX_FMT_YUYV:" << V4L2_PIX_FMT_YUYV;
    qDebug() << "V4l2::getVideoFmt"
             << " V4L2_PIX_FMT_UYVY:" << V4L2_PIX_FMT_UYVY;
}

int V4l2::setVideoFmt(int fd, int width, int height)
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    int result = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if (-1 == result)
    {
        qDebug() << "V4l2::setVideoFmt "
                 << " setVideoFmt failed:" << fmt.fmt.pix.pixelformat
                 << " errno:" << errno;
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
        qDebug() << "V4l2::getFps "
                 << " numerator:" << parm.parm.capture.timeperframe.numerator
                 << " denominator:" << parm.parm.capture.timeperframe.denominator
                 << " capturemode:" << parm.parm.capture.capturemode;
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
        qDebug() << "V4l2::setFps "
                 << " setFps failed:" << fps
                 << " errno:" << errno;
    }

    return result;
}

int V4l2::initV4l2Buf(int fd, int fd_ipu, struct buffer* v4l2_buf, unsigned int buf_count, v4l2_memory mem_type)
{
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = buf_count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = mem_type;
    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        qDebug() << "V4l2::initV4l2Buf"
                 << " REQBUFS failed";
        return -1;
    }

    if (req.count < buf_count) {
        qDebug() << "V4l2::initV4l2Buf"
                 << " Insufficient buffer memory on device";
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
                qDebug() << "V4l2::initV4l2Buf"
                         << " QUERYBUF failed";
                return -1;
            }

            v4l2_buf[i].length = buf.length;
            v4l2_buf[i].offset = buf.m.offset;
            // 映射内存
            v4l2_buf[i].start = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
            if (MAP_FAILED == v4l2_buf[i].start)
            {
                qDebug() << "V4l2::initV4l2Buf"
                        << " mmap failed";
                return -1;
            }
            memset(v4l2_buf[i].start, 0xFF, v4l2_buf[i].length);
        }
    }
    else if (mem_type == V4L2_MEMORY_USERPTR)
    {
#if USE_IMX_IPU
        for (unsigned int i = 0; i < req.count; ++i)
        {
            //unsigned int page_size = getpagesize();
            unsigned int buf_size = v4l2_buf[i].width * v4l2_buf[i].height * 2;
           // buf_size = (buf_size + page_size - 1) & ~(page_size - 1);
            v4l2_buf[i].length = v4l2_buf[i].offset = buf_size;
            if (-1 == ioctl(fd_ipu, IPU_ALLOC, &v4l2_buf[i].offset))
            {
                qDebug() << "V4l2::initV4l2Buf"
                        << " IPU_ALLOC failed";
                return -1;
            }

            v4l2_buf[i].start = mmap(0, buf_size, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd_ipu, v4l2_buf[i].offset);
            if (NULL == v4l2_buf[i].start)
            {
                qDebug() << "V4l2::initV4l2Buf"
                        << " ipu map failed";
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
                qDebug() << "V4l2::initV4l2Buf"
                        << " QUERYBUF failed";
                return -1;
            }
*/
        }
#endif
    }

    return 0;
}

int V4l2::initIpuBuf(int fd_ipu, struct buffer* ipu_buf, unsigned int buf_count)
{
#if USE_IMX_IPU
    for (unsigned int i = 0; i < buf_count; ++i)
    {
        //unsigned int page_size = getpagesize();
        unsigned int buf_size = ipu_buf[i].width * ipu_buf[i].height * 3;
        //buf_size = (buf_size + page_size - 1) & ~(page_size - 1);
        ipu_buf[i].length = ipu_buf[i].offset = buf_size;
        if (-1 == ioctl(fd_ipu, IPU_ALLOC, &ipu_buf[i].offset))
        {
            qDebug() << "V4l2::initIpuBuf"
                    << " IPU_ALLOC failed";
            return -1;
        }

        ipu_buf[i].start = mmap(0, buf_size, PROT_READ | PROT_WRITE,
                                MAP_SHARED, fd_ipu, ipu_buf[i].offset);
        if (NULL == ipu_buf[i].start)
        {
            qDebug() << "V4l2::initIpuBuf"
                    << " ipu map failed";
            return -1;
        }

        qDebug() << "V4l2::initIpuBuf"
                 << " width:" << ipu_buf[i].width
                 << " height:" << ipu_buf[i].height
                 << " buf_size:" << buf_size
                 << " IPU_ALLOC ok";
    }

#endif
    return 0;
}

int V4l2::startCapture(int fd, struct buffer* v4l2_buf, v4l2_memory mem_type)
{
    unsigned int i = 0;
    for (i = 0; i < V4L2_BUF_COUNT; ++i)
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
            qDebug() << "V4l2::startCapture"
                    << " QBUF failed";
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

void V4l2::v4l2QBuf(int fd, struct v4l2_buffer* buf)
{
    ioctl(fd, VIDIOC_QBUF, buf); //再将其入列
}
