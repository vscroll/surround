#ifndef GLRENDER3DWORKER_H
#define GLRENDER3DWORKER_H

#include "wrap_thread.h"

class ICapture;
class GLRenderWindow;
class GLShader;
class GLRender3DWorker : public WrapThread
{
public:
    GLRender3DWorker();
    virtual ~GLRender3DWorker();

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
