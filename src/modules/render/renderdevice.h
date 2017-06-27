#ifndef RENDERDEVICE_H
#define RENDERDEVICE_H

#include <linux/mxcfb.h>

typedef struct render_surface_t {
    unsigned char* srcBuf;
    unsigned int srcPixfmt;
    unsigned int srcLeft;
    unsigned int srcTop;
	unsigned int srcStride;
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
    RenderDevice(unsigned int devIndex, bool blank);
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

private:
    unsigned int mDevIndex;
    bool mBlank;

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
};

#endif // RENDERDEVICE_H
