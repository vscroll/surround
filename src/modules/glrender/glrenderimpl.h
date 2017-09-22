#ifndef GLRENDERIMPL_H
#define GLRENDERIMPL_H

#include "IGLRender.h"

class IGLRenderWorker;
class GLRenderImpl : public IGLRender
{
public:
    GLRenderImpl();
    virtual ~GLRenderImpl();
    virtual void setDisplayMode(unsigned int displayMode);
    virtual void setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    virtual void setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);

    virtual void updateFocusChannel();
    virtual void updatePanoramaView();

    virtual int init(ICapture* capture);
    virtual int start(unsigned int fps);
    virtual void stop();

    virtual void draw();

private:
    IGLRenderWorker *mWorker;
};

#endif // GLRENDERIMPL_H
