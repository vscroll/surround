#ifndef GLRENDERWORKER_H
#define GLRENDERWORKER_H

#include "wrap_thread.h"

class ICapture;
class GLRenderWindow;
class GLShader;
class GLRenderWorker : public WrapThread
{
public:
    GLRenderWorker();
    virtual ~GLRenderWorker();

    void setDisplayMode(unsigned int displayMode);
    void setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    void setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    int init(ICapture* capture);

    void draw();
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
