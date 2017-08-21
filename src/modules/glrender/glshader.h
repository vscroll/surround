#ifndef GLSHADERR_H
#define GLSHADERR_H

#include "esUtil.h"

class GLShader
{
public:
    GLShader(ESContext* context);
    virtual ~GLShader();

    virtual const char* getVertShader() = 0;
    virtual const char* getFragShader() = 0;
    virtual int initConfig() = 0;
    virtual int initProgram();
    virtual void initVertex() = 0;
    virtual void initTexture() = 0;
    virtual void draw() = 0;
    virtual void shutdown();

protected:
    void checkGlError(const char* op);

protected:
    ESContext* mESContext;
    GLuint mProgramObject;
};

#endif // GLSHADERR_H
