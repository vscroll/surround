#ifndef GLRENDERWORKER_H
#define GLRENDERWORKER_H

#include "IGLRenderWorker.h"
#include "wrap_thread.h"

class ICapture;
class GLRenderWindow;
class GLShader;
class GLRenderWorker : public IGLRenderWorker, public WrapThread
{
public:
    GLRenderWorker();
    virtual ~GLRenderWorker();

    virtual void setDisplayMode(unsigned int displayMode);
    virtual void setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    virtual void setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    virtual int init(ICapture* capture);

    virtual void draw();
public:
    virtual void run();

private:
    unsigned int mDisplayMode;

    unsigned int mPanoramaViewleft;
    unsigned int mPanoramaViewTop;
    unsigned int mPanoramaViewWidth;
    unsigned int mPanoramaViewHeight;

    unsigned int mXViewleft;
    unsigned int mXViewTop;
    unsigned int mXViewWidth;
    unsigned int mXViewHeight;

    GLRenderWindow* mWindow;
    GLShader* mShader;
};

#endif // GLRENDERWORKER_H
