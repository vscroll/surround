#ifndef UTIL_20170427_H
#define UTIL_20170427_H

#include <stdio.h>
#include <sys/time.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Util
{
public:
    Util();

    static void yuyv_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);
    static void uyvy_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);
    static void uyvy_to_yuv(int width, int height, unsigned char *uyvy, unsigned char *y, unsigned char *u, unsigned char *v);
    static void yuyv_to_yuv(int width, int height, unsigned char *yuyv, unsigned char *y, unsigned char *u, unsigned char *v);
    static void yuyv_to_yuv(int width, int height, unsigned char *yuyv, unsigned char *y, unsigned char *uv);

    static inline long get_system_milliseconds()
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return tv.tv_sec*1000 + tv.tv_usec/1000;;
    }

    static int getAbsolutePath(char* path, int length);

    static bool exists(const char* filename);

    static int readAllBytes(std::vector<unsigned char>& byteArray, const char* filename);

    static int writeAllBytes(const char* filename, const std::vector<unsigned char>& byteArray, int size);
private:
    static size_t convert(const std::streamoff value);
    static int getStreamLength(std::ifstream& stream);
    static int streamRead(std::ifstream& stream, void* dst, const size_t length);
    static int streamWrite(std::ofstream& stream, const void* dst, const size_t length);
};

#endif // UTIL_H
