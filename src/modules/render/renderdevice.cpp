#include "renderdevice.h"
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include "g2d.h"

RenderDevice::RenderDevice()
{
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

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        mG2dbuf[i] = NULL;
    }

    mG2dBufIndex = 0;
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

    if (openG2d() < 0)
    {
        closeFramebuffer();
        closeG2d();
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
    closeG2d();
}

int RenderDevice::openFramebuffer()
{
    if ((mFBFd = open("/dev/fb0", O_RDWR, 0)) < 0) {
	    return -1;
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
              << " xres_virtual:" << mScreenInfo.xres_virtual
              << " yres_virtual:" << mScreenInfo.yres_virtual
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

int RenderDevice::openG2d()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
#if CACHEABLE
        mG2dbuf[i] = g2d_alloc(mDstWidth*mDstHeight*3, 1);
#else
        mG2dbuf[i] = g2d_alloc(mDstWidth*mDstHeight*3, 0);
#endif
        if(NULL == mG2dbuf[i])
        {
            std::cout << "Fail to allocate physical memory for image buffer"
                      << std::endl;
            return -1;
        }
    }

    std::cout << "RenderDevice::openG2d ok"
              << std::endl;
}

void RenderDevice::closeG2d()
{
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
	    if (mG2dbuf[i] != NULL)
        {
            g2d_free(mG2dbuf[i]);
            mG2dbuf[i] = NULL;
        }
    }
}

void RenderDevice::drawImage(unsigned char* buf,
            unsigned int srcPixfmt,
            unsigned int srcWidth,
            unsigned int srcHeight,
            unsigned int srcSize,
            unsigned int dstLeft,
		    unsigned int dstTop,
		    unsigned int dstWidth,
		    unsigned int dstHeight)
{
    void *g2dHandle;
    if (g2d_open(&g2dHandle) == -1
	    || g2dHandle == NULL)
    {
        std::cout << "Fail to open g2d device"
                  << std::endl;
        return;
    }

    if ((dstLeft + dstWidth) > (int)mScreenInfo.xres || (dstTop + dstHeight) > (int)mScreenInfo.yres)  {
        std::cout << "Bad display image dimensions"
                << ", dst left:" << dstLeft
                << ", dst top:" << dstTop
                << ", dst width:" << dstWidth
                << ", dst height:" << dstHeight
                << std::endl;
        return;
    }

    struct g2d_buf* g2dbuf = mG2dbuf[mG2dBufIndex];
    if (++mG2dBufIndex >= BUFFER_SIZE)
    {
        mG2dBufIndex = 0;
    }

    memcpy(g2dbuf->buf_vaddr, buf, srcSize);
    struct g2d_surface src;
    struct g2d_surface dst;

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

    src.format = G2D_UYVY;
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
            src.planes[1] = g2dbuf->buf_paddr + srcWidth * srcHeight;
            break;
        case G2D_I420:
            src.planes[0] = g2dbuf->buf_paddr;
            src.planes[1] =g2dbuf->buf_paddr + srcWidth * srcHeight;
            src.planes[2] = src.planes[1]  + srcWidth * srcHeight / 4;
            break;
        case G2D_NV16:
            src.planes[0] = g2dbuf->buf_paddr;
            src.planes[1] = g2dbuf->buf_paddr + srcWidth * srcHeight;
            break;
        default:
            std::cout << "Unsupport image format"
                    << std::endl;
            return;
    }

    src.left = 0;
    src.top = 0;
    src.right = srcWidth;
    src.bottom = srcHeight;
    src.stride = srcWidth;
    src.width  = srcWidth;
    src.height = srcHeight;
    src.rot  = G2D_ROTATION_0;

    dst.planes[0] = mFBPhys;
    dst.left = dstLeft;
    dst.top = dstTop;
    dst.right = dstLeft + dstWidth;
    dst.bottom = dstTop + dstHeight;
    dst.stride = mScreenInfo.xres;
    dst.width  = mScreenInfo.xres;
    dst.height = mScreenInfo.yres;
    dst.rot    = G2D_ROTATION_0;
    dst.format = mScreenInfo.bits_per_pixel == 16 ? G2D_RGB565 : (mScreenInfo.red.offset == 0 ? G2D_RGBA8888 : G2D_BGRA8888);
#if 0
    if (set_alpha)
    {
        src.blendfunc = G2D_ONE;
        dst.blendfunc = G2D_ONE_MINUS_SRC_ALPHA;

        src.global_alpha = 0x80;
        dst.global_alpha = 0xff;

        g2d_enable(g2dHandle, G2D_BLEND);
        g2d_enable(g2dHandle, G2D_GLOBAL_ALPHA);
    }
#endif

    g2d_blit(g2dHandle, &src, &dst);
    g2d_finish(g2dHandle);
#if 0
    if (set_alpha)
    {
        g2d_disable(g2dHandle, G2D_GLOBAL_ALPHA);
        g2d_disable(g2dHandle, G2D_BLEND);
    }
#endif
    g2d_close(g2dHandle);

}
