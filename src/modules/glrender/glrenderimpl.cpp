#include "glrenderimpl.h"
#include "IGLRenderWorker.h"
#include "glrenderworker.h"

GLRenderImpl::GLRenderImpl()
{
    mWorker = new GLRenderWorker();
}

GLRenderImpl::~GLRenderImpl()
{
    if (NULL != mWorker)
    {
        delete mWorker;
        mWorker = NULL;
    }
}

void GLRenderImpl::setDisplayMode(unsigned int displayMode)
{
    if (NULL == mWorker)
    {
        return;
    }

    mWorker->setDisplayMode(displayMode);
}

void GLRenderImpl::setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{
    if (NULL == mWorker)
    {
        return;
    }

    mWorker->setPanoramaViewRect(left, top, width, height);
}

void GLRenderImpl::setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{
    if (NULL == mWorker)
    {
        return;
    }

    mWorker->setXViewRect(left, top, width, height);
}

int GLRenderImpl::init(ICapture* capture)
{
    if (NULL == mWorker)
    {
        return -1;
    }

    return mWorker->init(capture);
}

int GLRenderImpl::start(unsigned int fps)
{
    if (NULL == mWorker)
    {
        return -1;
    }

    //return mWorker->start(fps);
}

void GLRenderImpl::stop()
{
    if (NULL != mWorker)
    {
        //mWorker->stop();
    }
}

void GLRenderImpl::draw()
{
    if (NULL != mWorker)
    {
        mWorker->draw();
    }
}
