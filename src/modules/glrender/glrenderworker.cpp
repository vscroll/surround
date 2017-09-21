#include "glrenderworker.h"
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "util.h"
#include "glrenderwindow.h"
#include "IGLRender.h"
#include "glshader.h"
#include "glshaderyuv.h"
#include "glshaderrgb.h"

GLRenderWorker::GLRenderWorker()
{
    mDisplayMode = IGLRender::DISPLAY_MODE_PANO_PLUS_FRONT;

    mPanoramaViewleft = 0;
    mPanoramaViewTop = 0;
    mPanoramaViewWidth = 0;
    mPanoramaViewHeight = 0;

    mXViewleft = 0;
    mXViewTop = 0;
    mXViewWidth = 0;
    mXViewHeight = 0;

    mWindow = NULL;
    mShader = NULL;
}

GLRenderWorker::~GLRenderWorker()
{

}

void GLRenderWorker::setDisplayMode(unsigned int displayMode)
{
    if (displayMode >= IGLRender::DISPLAY_MODE_MIN
        && displayMode <= IGLRender::DISPLAY_MODE_MAX)
    {
        mDisplayMode = displayMode;
    }
}

void GLRenderWorker::setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{
}

void GLRenderWorker::setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{

}

int GLRenderWorker::init(ICapture* capture)
{
    if (NULL == mWindow)
    {
        mWindow = new GLRenderWindow();
    }

    if (mWindow->create(0) < 0)
    {
        return -1;
    }

    if (NULL == mShader)
    {
#if 0
        mShader = new GLShaderYUV(&mWindow->mESContext, "panorama_yuv.prog", capture);
#else
        mShader = new GLShaderRGB(&mWindow->mESContext, "panorama_rgb.prog", capture);
#endif
    }

    mShader->initConfig();
    mShader->initProgram();
    mShader->initVertex();
    mShader->initTexture();

    return 0;
}

void GLRenderWorker::draw()
{
    //all the opengl es functions must be called in one thread
    if (NULL != mShader)
    {
        mShader->draw();
    }
}

void GLRenderWorker::run()
{

}
