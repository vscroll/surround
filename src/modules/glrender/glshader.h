#ifndef GLSHADERR_H
#define GLSHADERR_H

#include <linux/mxcfb.h>
#include "esUtil.h"

class GLShader
{
public:
    GLShader();
    virtual ~GLShader();

    virtual const char* getVertShader() = 0;
    virtual const char* getFragShader() = 0;
    virtual int initProgram();
    virtual void initVertex() = 0;
    virtual void initTexture() = 0;
    virtual void draw() = 0;
    virtual void shutdown();

protected:
    void checkGlError(const char* op);

private:
    int getScreenInfo(int devIndex);

protected:
    struct fb_var_screeninfo mScreenInfo;

    ESContext mESContext;
    GLuint mProgramObject;
};

#endif // GLSHADERR_H
