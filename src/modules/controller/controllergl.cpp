#include "controllergl.h"
#include <iostream>
#include "util.h"
#include "IConfig.h"
#include "ICapture.h"
#include "IGLRender.h"
#include "glrenderimpl.h"

ControllerGL::ControllerGL()
{
    mGLRender = NULL;
}

ControllerGL::~ControllerGL()
{
}

int ControllerGL::startGLRenderModule()
{
    if (NULL == mConfig
        || NULL == mCapture)
    {
        return -1;
    }

    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    mGLRender = new GLRenderImpl();
    if (mGLRender->init(mCapture) < 0)
    {
        return -1;
    }

    mGLRender->start(VIDEO_FPS_15);

    return 0;
}

void ControllerGL::stopGLRenderModule()
{
    if (NULL != mGLRender)
    {
        mGLRender->stop();
        delete mGLRender;
        mGLRender = NULL;
    }  
}
