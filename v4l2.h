#ifndef V4L2_H
#define V4L2_H

#include <linux/videodev2.h>

class V4l2
{
public:
    V4l2();

    static const unsigned int V4L2_BUF_COUNT = 1;

    struct buffer
    {
        void* start;
        unsigned int length;
    };

    static void getVideoCap(int fd);
    static void getVideoFmt(int fd, int* width, int* height);
    static int setVideoFmt(int fd, int width, int height);
    static int getFps(int fd);
    static int setFps(int fd, int fps);
    static int initV4l2Buf(int fd, struct buffer** v4l2_buf);
    static int startCapture(int fd);
    static void stoptCapture(int fd);
    static int readFrame(int fd, struct v4l2_buffer* buf);
    static void v4l2QBuf(int fd, struct v4l2_buffer* buf);
};

#endif // V4L2_H
