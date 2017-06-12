#include "renderdevice.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include "g2d.h"

#define CACHEABLE 0

RenderDevice::RenderDevice(unsigned int devIndex, bool blank)
{
    mDevIndex = devIndex;
    mBlank = blank;

    mDstLeft = 0;
    mDstTop = 0;
    mDstWidth = 0;
    mDstHeight = 0;

    mFBFd = -1;
    memset(&mFBInfo, 0, sizeof(mFBInfo));
    memset(&mScreenInfo, 0, sizeof(mScreenInfo));

    mFBPhys = 0;;
    mFBMem = NULL;
    mFBSize = 0;
}

RenderDevice::~RenderDevice()
{
}

int RenderDevice::openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight)
{
    mDstLeft = dstLeft;
    mDstTop = dstTop;
    mDstWidth = dstWidth;
    mDstHeight = dstHeight;

    if (openFramebuffer() < 0)
    {
        closeFramebuffer();
	    return -1;
    }

    std::cout << "openDevice ok"
            << ", dst left:" << mDstLeft
            << ", dst top:" << mDstTop
            << ", dst width:" << mDstWidth
            << ", dst height:" << mDstHeight
            << std::endl;

    return 0;
}

void RenderDevice::closeDevice()
{

    closeFramebuffer();
}

int RenderDevice::openFramebuffer()
{
    char fb[16] = {0};
    sprintf(fb, "/dev/fb%d", mDevIndex);
    if ((mFBFd = open(fb, O_RDWR, 0)) < 0) {
	    return -1;
    }

    if (!mBlank)
    {
        if (ioctl(mFBFd, FBIOBLANK, FB_BLANK_UNBLANK) < 0)
        {
            std::cout << "FBIOBLANK failed"
                    << std::endl;
	        return -1;
        }
    }

    /* Get fix screen info. */
    if (ioctl(mFBFd, FBIOGET_FSCREENINFO, &mFBInfo) < 0) {
        std::cout << "FBIOGET_FSCREENINFO failed"
                  << std::endl;
	    return -1;
    }

    /* Get variable screen info. */
    if (ioctl(mFBFd, FBIOGET_VSCREENINFO, &mScreenInfo) < 0) {
        std::cout << "FBIOGET_VSCREENINFO failed"
                  << std::endl;
	    return -1;
    }

    mFBPhys = mFBInfo.smem_start + (mScreenInfo.xres_virtual * mScreenInfo.yoffset * mScreenInfo.bits_per_pixel / 8);
    mFBSize = mScreenInfo.xres_virtual * mScreenInfo.yres_virtual * mScreenInfo.bits_per_pixel / 8;

    /* Map the device to memory*/
    mFBMem = (unsigned short *)mmap(0, mFBSize, PROT_READ | PROT_WRITE, MAP_SHARED, mFBFd, 0);
    if ((int)mFBMem <= 0) {
        std::cout << "failed to map framebuffer device to memory"
                  << std::endl;
	    return -1;
    }

    std::cout << "RenderDevice::openDevice"
              << " fb:" << fb
              << " xres_virtual:" << mScreenInfo.xres_virtual
              << " yres_virtual:" << mScreenInfo.yres_virtual
              << ", xres:" << mScreenInfo.xres
              << ", yres:" << mScreenInfo.yres
              << std::endl;

    return 0;
}

void RenderDevice::closeFramebuffer()
{
    if (mFBFd > 0)
    {
        munmap(mFBMem, mFBSize);         
        close(mFBFd);
        mFBFd = -1;
    }
}

void RenderDevice::drawImage(struct render_surface_t* surface, bool alpha)
{
    if ((surface->dstLeft + surface->dstWidth) > (int)mScreenInfo.xres || (surface->dstTop + surface->dstHeight) > (int)mScreenInfo.yres)  {
        std::cout << "Bad display image dimensions"
                << ", dst left:" << surface->dstLeft
                << ", dst top:" << surface->dstTop
                << ", dst width:" << surface->dstWidth
                << ", dst height:" << surface->dstHeight
                << ", xres:" << mScreenInfo.xres
                << ", yres:" << mScreenInfo.yres
                << std::endl;
        return;
    }

    struct g2d_surface src;
    struct g2d_surface dst;
    if (surface->srcPixfmt == V4L2_PIX_FMT_UYVY)
    {
        src.format = G2D_UYVY;
    }
    else if (surface->srcPixfmt == V4L2_PIX_FMT_YUYV)
    {
        src.format = G2D_YUYV;
    }

    struct g2d_buf* g2dbuf = NULL;
#if CACHEABLE
    g2dbuf = g2d_alloc(surface->srcSize, 1);
#else
    g2dbuf = g2d_alloc(surface->srcSize, 0);
#endif

    memcpy(g2dbuf->buf_vaddr, surface->srcBuf, surface->srcSize);

#if CACHEABLE
    g2d_cache_op(g2dbuf, G2D_CACHE_FLUSH);
#endif

    /*
NOTE: in this example, all the test image data meet with the alignment requirement.
Thus, in your code, you need to pay attention on that.

Pixel buffer address alignment requirement,
RGB/BGR:  pixel data in planes [0] with 16bytes alignment,
NV12/NV16:  Y in planes [0], UV in planes [1], with 64bytes alignment,
I420:    Y in planes [0], U in planes [1], V in planes [2], with 64 bytes alignment,
YV12:  Y in planes [0], V in planes [1], U in planes [2], with 64 bytes alignment,
NV21/NV61:  Y in planes [0], VU in planes [1], with 64bytes alignment,
YUYV/YVYU/UYVY/VYUY:  in planes[0], buffer address is with 16bytes alignment.
*/
    switch (src.format) {
        case G2D_RGB565:
        case G2D_RGBA8888:
        case G2D_RGBX8888:
        case G2D_BGRA8888:
        case G2D_BGRX8888:
        case G2D_BGR565:
        case G2D_YUYV:
        case G2D_UYVY:
            src.planes[0] = g2dbuf->buf_paddr;
            break;
        case G2D_NV12:
            src.planes[0] = g2dbuf->buf_paddr;
            src.planes[1] = g2dbuf->buf_paddr + surface->srcWidth * surface->srcHeight;
            break;
        case G2D_I420:
            src.planes[0] = g2dbuf->buf_paddr;
            src.planes[1] = g2dbuf->buf_paddr + surface->srcWidth * surface->srcHeight;
            src.planes[2] = src.planes[1]  + surface->srcWidth * surface->srcHeight / 4;
            break;
        case G2D_NV16:
            src.planes[0] = g2dbuf->buf_paddr;
            src.planes[1] = g2dbuf->buf_paddr + surface->srcWidth * surface->srcHeight;
            break;
        default:
            std::cout << "Unsupport image format"
                    << std::endl;
            return;
    }

    src.left = 0;
    src.top = 0;
    src.right = surface->srcWidth;
    src.bottom = surface->srcHeight;
    src.stride = surface->srcWidth;
    src.width  = surface->srcWidth;
    src.height = surface->srcHeight;
    src.rot  = G2D_ROTATION_0;

    dst.planes[0] = mFBPhys;
    dst.left = surface->dstLeft;
    dst.top = surface->dstTop;
    dst.right = surface->dstLeft + surface->dstWidth;
    dst.bottom = surface->dstTop + surface->dstHeight;
    dst.stride = mScreenInfo.xres;
    dst.width  = mScreenInfo.xres;
    dst.height = mScreenInfo.yres;
    dst.rot    = G2D_ROTATION_0;
    dst.format = mScreenInfo.bits_per_pixel == 16 ? G2D_RGB565 : (mScreenInfo.red.offset == 0 ? G2D_RGBA8888 : G2D_BGRA8888);

    void *g2dHandle;
    if (g2d_open(&g2dHandle) == -1
	    || g2dHandle == NULL)
    {
        std::cout << "Fail to open g2d device"
                  << std::endl;
        return;
    }

    if (alpha)
    {
        src.blendfunc = G2D_ONE;
        src.global_alpha = 0x80;

        dst.blendfunc = G2D_ONE_MINUS_SRC_ALPHA;
        dst.global_alpha = 0xff;

        g2d_enable(g2dHandle, G2D_BLEND);
        g2d_enable(g2dHandle, G2D_GLOBAL_ALPHA);
    }

    g2d_blit(g2dHandle, &src, &dst);
    g2d_finish(g2dHandle);

    if (alpha)
    {
        g2d_disable(g2dHandle, G2D_GLOBAL_ALPHA);
        g2d_disable(g2dHandle, G2D_BLEND);
    }

    g2d_close(g2dHandle);
    g2d_free(g2dbuf);
}

void RenderDevice::drawMultiImages(struct render_surface_t surfaces[], unsigned int num, bool alpha)
{
#if 1
    void *g2dHandle;
    if (g2d_open(&g2dHandle) == -1
	    || g2dHandle == NULL)
    {
        std::cout << "Fail to open g2d device"
                  << std::endl;
        return;
    }

    struct g2d_buf* mul_s_buf[num] = {NULL};

    for (unsigned int i = 0; i < num; ++i)
    {
        struct g2d_surface src;
        struct g2d_surface dst;
        if (surfaces[i].srcPixfmt == V4L2_PIX_FMT_UYVY)
        {
            src.format = G2D_UYVY;
        }
        else if (surfaces[i].srcPixfmt == V4L2_PIX_FMT_YUYV)
        {
            src.format = G2D_YUYV;
        }
#if CACHEABLE
        mul_s_buf[i] = g2d_alloc(surfaces[i].srcSize, 1);
#else
        mul_s_buf[i] = g2d_alloc(surfaces[i].srcSize, 0);
#endif
        memcpy(mul_s_buf[i]->buf_vaddr, surfaces[i].srcBuf, surfaces[i].srcSize);

#if CACHEABLE
        g2d_cache_op(mul_s_buf[i], G2D_CACHE_FLUSH);
#endif

        src.planes[0] = mul_s_buf[i]->buf_paddr;
        src.left = 0;
        src.top = 0;
        src.right = surfaces[i].srcWidth;
        src.bottom = surfaces[i].srcHeight;
        src.stride = surfaces[i].srcWidth;
        src.width  = surfaces[i].srcWidth;
        src.height = surfaces[i].srcHeight;
        src.rot  = G2D_ROTATION_0;

        dst.planes[0] = mFBPhys;
        dst.left = surfaces[i].dstLeft;
        dst.top = surfaces[i].dstTop;
        dst.right = surfaces[i].dstLeft + surfaces[i].dstWidth;
        dst.bottom = surfaces[i].dstTop + surfaces[i].dstHeight;
        dst.stride = mScreenInfo.xres;
        dst.width  = mScreenInfo.xres;
        dst.height = mScreenInfo.yres;
        dst.rot    = G2D_ROTATION_0;
        dst.format = mScreenInfo.bits_per_pixel == 16 ? G2D_RGB565 : (mScreenInfo.red.offset == 0 ? G2D_RGBA8888 : G2D_BGRA8888);

        if (alpha)
        {
            src.blendfunc = G2D_ONE;
            src.global_alpha = 0x80;

            dst.blendfunc = G2D_ONE_MINUS_SRC_ALPHA;
            dst.global_alpha = 0xff;

            g2d_enable(g2dHandle, G2D_BLEND);
            g2d_enable(g2dHandle, G2D_GLOBAL_ALPHA);
        }

        g2d_blit(g2dHandle, &src, &dst);

        if (alpha)
        {
            g2d_disable(g2dHandle, G2D_GLOBAL_ALPHA);
            g2d_disable(g2dHandle, G2D_BLEND);
        }
    }

    g2d_finish(g2dHandle);

    for (unsigned int i = 0; i < num; ++i)
    {
        g2d_free(mul_s_buf[i]);
    }

    g2d_close(g2dHandle);

#else
    void *g2dHandle;
    if (g2d_open(&g2dHandle) == -1
	    || g2dHandle == NULL)
    {
        std::cout << "Fail to open g2d device"
                  << std::endl;
        return;
    }

    struct g2d_buf* mul_s_buf[num];
    struct g2d_surface_pair* sp[num];

    for (unsigned int i = 0; i < num; ++i)
    {
        mul_s_buf[i] = g2d_alloc(surfaces[i].srcSize, 0);
        memcpy(mul_s_buf[i]->buf_vaddr, surfaces[i].srcBuf, surfaces[i].srcSize);

        sp[i] = (struct g2d_surface_pair*)malloc(sizeof(struct g2d_surface_pair));
        sp[i]->s.left = 0;
        sp[i]->s.top = 0;
        sp[i]->s.right = surfaces[i].srcWidth;
        sp[i]->s.bottom = surfaces[i].srcHeight;
        sp[i]->s.stride = surfaces[i].srcWidth;
        sp[i]->s.width = surfaces[i].srcWidth;
        sp[i]->s.height = surfaces[i].srcHeight;
        sp[i]->s.rot = G2D_ROTATION_0;
        sp[i]->s.format = G2D_UYVY;
        sp[i]->s.planes[0] = mul_s_buf[i]->buf_paddr;

        sp[i]->d.left = 0;
        sp[i]->d.top = 0;
        sp[i]->d.right = surfaces[i].dstWidth;
        sp[i]->d.bottom = surfaces[i].srcHeight;
        sp[i]->d.stride = surfaces[i].dstWidth;
        sp[i]->d.width = surfaces[i].dstWidth;
        sp[i]->d.height = surfaces[i].srcHeight;
        sp[i]->d.format = mScreenInfo.bits_per_pixel == 16 ? G2D_RGB565 : (mScreenInfo.red.offset == 0 ? G2D_RGBA8888 : G2D_BGRA8888);
        sp[i]->d.rot = G2D_ROTATION_0;
        sp[i]->d.planes[0] = mFBPhys;
    }

    g2d_multi_blit(g2dHandle, sp, num);
    g2d_finish(g2dHandle);

    for (unsigned int i = 0; i < num; ++i)
    {
        g2d_free(mul_s_buf[i]);
        free(sp[i]);
    }

    g2d_close(g2dHandle);
#endif

}
