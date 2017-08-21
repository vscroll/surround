#ifndef GLRENDER3DWORKER_H
#define GLRENDER3DWORKER_H

#include "IGLRenderWorker.h"
#include "wrap_thread.h"

class ICapture;
class GLRenderWindow;
class GLShader;
class GLRender3DWorker : public IGLRenderWorker, public WrapThread
{
public:
    GLRender3DWorker();
    virtual ~GLRender3DWorker();
    virtual void setDisplayMode(unsigned int displayMode);
    virtual void setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    virtual void setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    int init(ICapture* capture);

    void draw();
public:
    virtual void run();

private:

    GLRenderWindow* mWindow;    
    static const int MODEL_NUM = 2;
    GLShader* mShader[MODEL_NUM];
};

#endif // GLRENDER3DWORKER_H
