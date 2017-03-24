#include "util.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

Util::Util()
{
}


void Util::write2File(int channel, void* image)
{
    static int count = 0;
    IplImage* frame = (IplImage*)image;
    char outImageName[16] = {0};
    IplImage* outImage = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);
    // 将原图拷贝过来
    cvCopy(frame,outImage,NULL);

    //设置保存的图片名称和格式
    memset(outImageName, 0, sizeof(outImageName));
    sprintf(outImageName, "test_cam%d_%d.jpg", channel, count++);
    //保存图片
    cvSaveImage(outImageName, outImage, 0);
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

