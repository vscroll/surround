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

    static inline long get_system_milliseconds()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return tv.tv_sec*1000 + tv.tv_usec/1000;;
    }

    static int getAbsolutePath(char* path, int length);
};

#endif // UTIL_H
