#ifndef RENDERDEVICE_H
#define RENDERDEVICE_H

#include <linux/mxcfb.h>

struct g2d_buf;
class RenderDevice
{
public:
    RenderDevice();
    virtual ~RenderDevice();
    int openDevice(unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height);

    void closeDevice();

    void drawImage(unsigned char* buf, unsigned int size);

private:
    int openFramebuffer();
    void closeFramebuffer();

    int openG2d();
    void closeG2d();
private:
    unsigned int mLeft;
    unsigned int mTop;
    unsigned int mWidth;
    unsigned int mHeight;

    int mFBFd;
    struct fb_fix_screeninfo mFBInfo;
    struct fb_var_screeninfo mScreenInfo;
    int mFBPhys;
    unsigned short *mFBMem;
    int mFBSize;

#define BUFFER_SIZE 2
    void* mG2dHandle;
    struct g2d_buf* mG2dbuf[BUFFER_SIZE];
private:

};

#endif // RENDERDEVICE_H
