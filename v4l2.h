#ifndef V4L2_H
#define V4L2_H

#include <linux/videodev2.h>

class V4l2
{
public:
    V4l2();

    static const unsigned int V4L2_BUF_COUNT = 4;

    struct buffer
    {
        void* start;
        unsigned int offset;
        unsigned int length;
        unsigned int width;
        unsigned int height;
        unsigned int fmt;
    };

    static void getVideoCap(int fd);
    static void getVideoFmt(int fd, int* width, int* height);
    static int setVideoFmt(int fd, int width, int height);
    static int getFps(int fd);
    static int setFps(int fd, int fps);
    static int initV4l2Buf(int fd, int fd_ipu, struct buffer* v4l2_buf, v4l2_memory mem_type);
    static int initIpuBuf(int fd_ipu, struct buffer* ipu_buf, unsigned int buf_count);
    static int startCapture(int fd, struct buffer* v4l2_buf, v4l2_memory mem_type);
    static void stoptCapture(int fd);
    static int readFrame(int fd, struct v4l2_buffer* buf, v4l2_memory mem_type);
    static void v4l2QBuf(int fd, struct v4l2_buffer* buf);
};

#endif // V4L2_H
