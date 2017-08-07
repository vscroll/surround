#include "glshader.h"
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "util.h"
#include "IGLRender.h"
#include "ICapture.h"


GLShader::GLShader()
{
    memset(&mScreenInfo, 0, sizeof(mScreenInfo));
    mProgramObject = -1;
}

GLShader::~GLShader()
{

}

int GLShader::initProgram()
{
    getScreenInfo(0);

    esInitContext(&mESContext);

    if (!esCreateWindow(&mESContext, "MultiTexture", mScreenInfo.xres, mScreenInfo.yres, ES_WINDOW_RGB))
    {
        std::cout << "esCreateWindow failed"
                << std::endl;
        return -1;
    }

    // Load the shaders and get a linked program object
    mProgramObject = esLoadProgram(getVertShader(), getFragShader());

    // Use the program object
    glUseProgram (mProgramObject);

    return 0;
}

int GLShader::getScreenInfo(int devIndex)
{
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
    if (ioctl(fd, FBIOGET_VSCREENINFO, &mScreenInfo) < 0)
    {
        std::cout << "FBIOGET_VSCREENINFO failed"
                  << std::endl;
    }

    std::cout << "GLRenderWorker::getScreenInfo"
              << " fb:" << fb
              << " xres_virtual:" << mScreenInfo.xres_virtual
              << " yres_virtual:" << mScreenInfo.yres_virtual
              << ", xres:" << mScreenInfo.xres
              << ", yres:" << mScreenInfo.yres
              << std::endl;

    close(fd);
    return 0;
}

void GLShader::shutdown()
{
    // Delete program object
    glDeleteProgram(mProgramObject);
}

void GLShader::checkGlError(const char* op)   
{  
    GLint error;  
    for (error = glGetError(); error; error = glGetError())   
    {  
        std::cout << "error::after " << op << "() glError (0x" << error << ")"
            << std::endl;
    }  
}

