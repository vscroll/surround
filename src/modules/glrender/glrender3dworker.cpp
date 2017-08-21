#include "glrender3dworker.h"
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
#include "glshadercar.h"
#include "glshadersurround.h"
#include "esUtil.h"

GLRender3DWorker::GLRender3DWorker()
{
    for (int i = 0; i < MODEL_NUM; ++i)
    {
        mShader[i] = NULL;
    }

    mWindow = NULL;
}

GLRender3DWorker::~GLRender3DWorker()
{

}

void GLRender3DWorker::setDisplayMode(unsigned int displayMode)
{

}

void GLRender3DWorker::setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{
}

void GLRender3DWorker::setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{

}

int GLRender3DWorker::init(ICapture* capture)
{
    if (NULL == mWindow)
    {
        mWindow = new GLRenderWindow();
    }

    if (mWindow->create(0) < 0)
    {
        return -1;
    }

    if (NULL == mShader[0])
    {
        mShader[0] = new GLShaderCar(&mWindow->mESContext);
    }

    if (NULL == mShader[1])
    {
        mShader[1] = new GLShaderSurround(&mWindow->mESContext);
    }

    for (int i = 0; i < 2; ++i)
    {
        mShader[i]->initConfig();
        mShader[i]->initProgram();
        mShader[i]->initVertex();
        mShader[i]->initTexture();
    }

    return 0;
}

void GLRender3DWorker::draw()
{
    //all the opengl es functions must be called in one thread
    while (true)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (NULL != mShader[i])
            {
                mShader[i]->draw();
            }
        }

        eglSwapBuffers(mWindow->mESContext.eglDisplay, mWindow->mESContext.eglSurface);
    }
}

void GLRender3DWorker::run()
{

}
