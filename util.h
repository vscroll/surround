#ifndef UTIL_H
#define UTIL_H

class Util
{
public:
    Util();

    static void write2File(int channel, void* image);

    static void yuyv_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);
    static void uyvy_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);
};

#endif // UTIL_H
