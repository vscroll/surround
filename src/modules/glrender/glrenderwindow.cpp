#include "glrenderwindow.h"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/mxcfb.h>

GLRenderWindow::GLRenderWindow()
{
}

GLRenderWindow::~GLRenderWindow()
{

}

int GLRenderWindow::create(int devIndex)
{
    esInitContext(&mESContext);

    struct fb_var_screeninfo screenInfo;
    int fd;
    char fb[16] = {0};
    sprintf(fb, "/dev/fb%d", devIndex);
    if ((fd = open(fb, O_RDWR, 0)) < 0)
    {
        std::cout << "open failed:" << fb
                  << std::endl;
	    return -1;
    }

    /* Get variable screen info. */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &screenInfo) < 0)
    {
        std::cout << "FBIOGET_VSCREENINFO failed"
                  << std::endl;
        return -1;
    }

    std::cout << "createWindow"
              << " fb:" << fb
              << " xres_virtual:" << screenInfo.xres_virtual
              << " yres_virtual:" << screenInfo.yres_virtual
              << ", xres:" << screenInfo.xres
              << ", yres:" << screenInfo.yres
              << std::endl;

    close(fd);

    if (!esCreateWindow(&mESContext, "MultiTexture", screenInfo.xres, screenInfo.yres, ES_WINDOW_RGB))
    {
        std::cout << "createWindow failed"
                << std::endl;
        return -1;
    }

    return 0;
}
