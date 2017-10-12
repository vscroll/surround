#include "util.h"
#include <unistd.h>
#include <string.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

Util::Util()
{
}

/* convert from 4:2:2 YUYV interlaced to RGB24 */
 /* based on ccvt_yuyv_bgr32() from camstream */
 #define SAT(c) \
         if (c & (~255)) { if (c < 0) c = 0; else c = 255; }

void Util::yuyv_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst)
{
    unsigned char *s;
    unsigned char *d;
    int l, c;
    int r, g, b, cr, cg, cb, y1, y2;

    l = height;
    s = src;
    d = dst;
    while (l--) {
        c = width >> 1;
        while (c--) {
            y1 = *s++;
            cb = ((*s - 128) * 454) >> 8;
            cg = (*s++ - 128) * 88;
            y2 = *s++;
            cr = ((*s - 128) * 359) >> 8;
            cg = (cg + (*s++ - 128) * 183) >> 8;

            r = y1 + cr;
            b = y1 + cb;
            g = y1 - cg;
            SAT(r);
            SAT(g);
            SAT(b);

            *d++ = b;
            *d++ = g;
            *d++ = r;

            r = y2 + cr;
            b = y2 + cb;
            g = y2 - cg;
            SAT(r);
            SAT(g);
            SAT(b);

            *d++ = b;
            *d++ = g;
            *d++ = r;
        }
    }
}

void Util::uyvy_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst)
{
   unsigned char *s;
   unsigned char *d;
   int l, c;
   int r, g, b, cr, cg, cb, y1, y2;

   l = height;
   s = src;
   d = dst;
   while (l--) {
      c = width >> 1;
      while (c--) {
         cb = ((*s - 128) * 454) >> 8;
         cg = (*s++ - 128) * 88;
         y1 = *s++;
         cr = ((*s - 128) * 359) >> 8;
         cg = (cg + (*s++ - 128) * 183) >> 8;
         y2 = *s++;

         r = y1 + cr;
         b = y1 + cb;
         g = y1 - cg;
         SAT(r);
         SAT(g);
         SAT(b);

     *d++ = b;
     *d++ = g;
     *d++ = r;

         r = y2 + cr;
         b = y2 + cb;
         g = y2 - cg;
         SAT(r);
         SAT(g);
         SAT(b);

     *d++ = b;
     *d++ = g;
     *d++ = r;
      }
   }
}

void Util::uyvy_to_yuv(int width, int height, unsigned char *uyvy, unsigned char *y, unsigned char *u, unsigned char *v)
{
    unsigned char *s;
    int l, c;

    l = height;
    s = uyvy;
    while (l--) {   
        c = width >> 1;
        while (c--) {
            *u++ = *s++;
            *y++ = *s++;
            *v++ = *s++;
            *y++ = *s++;
        }
    }
}

void Util::yuyv_to_yuv(int width, int height, unsigned char *yuyv, unsigned char *y, unsigned char *u, unsigned char *v)
{
    unsigned char *s;
    int l, c;

    l = height;
    s = yuyv;
    while (l--) {   
        c = width >> 1;
        while (c--) {
            *y++ = *s++;
            *u++ = *s++;
            *y++ = *s++;
            *v++ = *s++;
        }
    }
}

void Util::yuyv_to_yuv(int width, int height, unsigned char *yuyv, unsigned char *y, unsigned char *uv)
{
    unsigned char *s;
    int l, c;

    l = height;
    s = yuyv;
    while (l--) {   
        c = width >> 1;
        while (c--) {
            *y++ = *s++;
            *uv++ = *s++;
            *y++ = *s++;
            *uv++ = *s++;
        }
    }
}

int Util::getAbsolutePath(char* path, int length)
{
    char buf[length] = {0}; 
    int count = readlink( "/proc/self/exe", buf, length);
    if (count < 0 || count >= length) 
    {
        perror("getAbsolutePath");
        return -1;
    }

    int i;
    for (i = count; i >=0; --i)
    {
        if (buf[i] == '/')
        {
            buf[i+1] = '\0';
            memcpy(path, buf, i+1);
            break;
        }
    }

    return 0;
}

bool Util::exists(const char* filename)
{
    std::ifstream file(filename);
    return file.good();
}

size_t Util::convert(const std::streamoff value)
{
    return static_cast<size_t>(value);
}

int Util::getStreamLength(std::ifstream& stream)
{
    if (!stream.good())
    {
        return -1;
    }

    // Dumb C++ way of getting the stream length
    stream.seekg(0, stream.end);
    const std::streamoff streamLength = stream.tellg();
    stream.seekg(0, stream.beg);

    return convert(streamLength);
}

int Util::streamRead(std::ifstream& stream, void* dst, const size_t length)
{
    // Read the entire content of the file
    if( length > 0 )
    {
        stream.read(static_cast<char*>(dst), length);
    }

    if (!stream.good())
    {
        return -1;
    }

    return 0;
}


int Util::streamWrite(std::ofstream& stream, const void* dst, const size_t length)
{
    // Read the entire content of the file
    if (length > 0)
    {
        stream.write(static_cast<const char*>(dst), length);
    }

    if (!stream.good())
    {
        return -1;
    }

    return 0;
}

int Util::readAllBytes(std::vector<unsigned char>& byteArray, const char* filename)
{
    try
    {
        std::ifstream file(filename, std::ios::in | std::ios::binary);
        const size_t length = getStreamLength(file);
        byteArray.resize(length);

        // Read the entire content of the file
        streamRead(file, byteArray.data(), length);
    }
    catch (const std::ios_base::failure& ex)
    {
        return -1;
    }

    return 0;
}

int Util::writeAllBytes(const char* filename, const std::vector<unsigned char>& byteArray, int size)
{
    try
    {
        std::ofstream file(filename, std::ios::out | std::ios::binary);
        // Write the entire content of the file
        streamWrite(file, byteArray.data(), size);
    }
    catch (const std::ios_base::failure& ex)
    {
        return -1;
    }
    return 0;
}


void Util::write2File(int channel, void* image)
{
    IplImage* frame = (IplImage*)image;
    char outImageName[256] = {0};
    IplImage* outImage = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);
    // 将原图拷贝过来
    cvCopy(frame,outImage,NULL);

    timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    tm now;
    localtime_r(&time.tv_sec, &now);

    //设置保存的图片名称和格式
    memset(outImageName, 0, sizeof(outImageName));
    sprintf(outImageName, "cam%d_%04d%02d%02d_%02d%02d%02d.jpg", channel,
            now.tm_year + 1900, now.tm_mon+1, now.tm_mday, 
            now.tm_hour, now.tm_min, now.tm_sec);
    //保存图片
    cvSaveImage(outImageName, outImage, 0);
}
