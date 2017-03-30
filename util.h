#ifndef UTIL_H
#define UTIL_H

#include <opencv/cv.h>
#include <QImage>

class Util
{
public:
    Util();

    static void write2File(int channel, void* image);

    static void yuyv_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);
    static void uyvy_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst);

    static QImage cvMat2QImage(const cv::Mat& mat);
    static cv::Mat QImage2cvMat(QImage& image);
};

#endif // UTIL_H
