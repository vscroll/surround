#ifndef UTIL_20170427_H
#define UTIL_20170427_H

#include <stdio.h>
#include <sys/time.h>

class Util
{
public:
    Util();

    static void yuyv_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);
    static void uyvy_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);
    static void uyvy_to_yuv(int width, int height, unsigned char *uyvy, unsigned char *y, unsigned char *u, unsigned char *v);
    static void yuyv_to_yuv(int width, int height, unsigned char *yuyv, unsigned char *y, unsigned char *u, unsigned char *v);

    static inline long get_system_milliseconds()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return tv.tv_sec*1000 + tv.tv_usec/1000;;
    }

    static int getAbsolutePath(char* path, int length);
};

#endif // UTIL_H
