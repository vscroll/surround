#ifndef RENDERDEVICE_H
#define RENDERDEVICE_H

#include <linux/mxcfb.h>

typedef struct render_surface_t {
    unsigned char* srcBuf;
    unsigned int srcPixfmt;
    unsigned int srcWidth;
    unsigned int srcHeight;
    unsigned int srcSize;
    unsigned int dstLeft;
	unsigned int dstTop;
	unsigned int dstWidth;
	unsigned int dstHeight;
} render_surface_t;

struct g2d_buf;
class RenderDevice
{
public:
    RenderDevice();
    virtual ~RenderDevice();
    int openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight);

    void closeDevice();

    void drawImage(struct render_surface_t* surface, bool alpha);

    void drawMultiImages(struct render_surface_t surfaces[], unsigned int num, bool alpha);

    unsigned int getDstLeft() { return mDstLeft; }
    unsigned int getDstTop() { return mDstTop; }
    unsigned int getDstWidth() { return mDstWidth; }
    unsigned int getDstHeight() { return mDstHeight; }
private:
    int openFramebuffer();
    void closeFramebuffer();

    int openG2d();
    void closeG2d();
private:
    unsigned int mDstLeft;
    unsigned int mDstTop;
    unsigned int mDstWidth;
    unsigned int mDstHeight;

    int mFBFd;
    struct fb_fix_screeninfo mFBInfo;
    struct fb_var_screeninfo mScreenInfo;
    int mFBPhys;
    unsigned short *mFBMem;
    int mFBSize;

#define BUFFER_SIZE 2
    struct g2d_buf* mG2dbuf[BUFFER_SIZE];

    unsigned int mG2dBufIndex;
private:

};

#endif // RENDERDEVICE_H
