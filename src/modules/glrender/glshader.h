#ifndef GLSHADERR_H
#define GLSHADERR_H

#include "esUtil.h"
#include <string>

class GLShader
{
public:
    GLShader(ESContext* context, const std::string programBinaryFile);
    virtual ~GLShader();

    virtual const char* getVertShader() = 0;
    virtual const char* getFragShader() = 0;
    virtual int initConfig() = 0;
    virtual int initProgram();
    virtual void initVertex() = 0;
    virtual void initTexture() = 0;
    virtual void draw() = 0;
    virtual void shutdown();

    virtual void updateFocusChannel() = 0;
    virtual void updatePanoramaView() = 0;
private:
    GLuint LoadProgram(unsigned char *buf, int length);

protected:
    void checkGlError(const char* op);

protected:
    ESContext* mESContext;
    std::string mProgramBinaryFile;
    GLuint mProgramObject;
};

#endif // GLSHADERR_H
