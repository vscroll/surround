#ifndef V4L2_H
#define V4L2_H

#include <linux/videodev2.h>

class V4l2
{
public:
    V4l2();

    struct buffer
    {
        void* start;
        unsigned int offset;
        unsigned int length;
        unsigned int width;
        unsigned int height;
        unsigned int pixfmt;
    };

    static void getVideoCap(int fd);
    static void getVideoFmt(int fd, unsigned int* pixfmt, unsigned int* width, unsigned int* height);
    static int setVideoFmt(int fd, unsigned int pixfmt, unsigned int width, unsigned int height);
    static int getFps(int fd);
    static int setFps(int fd, int fps);
    static int v4l2ReqBuf(int fd, struct buffer* v4l2_buf, unsigned int buf_count, v4l2_memory mem_type, int fd_ipu, unsigned int frame_size);
    static int v4l2AllocBuf(int fd, struct buffer* v4l2_buf, unsigned int buf_count, v4l2_memory mem_type, unsigned int frame_size);
    static int startCapture(int fd, struct buffer* v4l2_buf, v4l2_memory mem_type);
    static void stoptCapture(int fd);
    static int readFrame(int fd, struct v4l2_buffer* buf, v4l2_memory mem_type);
    static void v4l2QueueBuf(int fd, struct v4l2_buffer* buf);
};

#endif // V4L2_H
