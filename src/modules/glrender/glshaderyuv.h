#ifndef GLSHADERYUV_H
#define GLSHADERYUV_H

#include <time.h>
#include <opencv/cv.h>
#include "common.h"
#include "glshader.h"

class ICapture;
class GLShaderYUV: public GLShader
{
public:
    GLShaderYUV(ESContext* context, const std::string programBinaryFile, ICapture* capture);
    virtual ~GLShaderYUV();

    virtual const char* getVertShader();
    virtual const char* getFragShader();
    virtual int initConfig();
    virtual void initVertex();
    virtual void initTexture();
    virtual void draw();
    virtual void shutdown();

private:
    GLboolean loadTexture(GLuint textureId, unsigned char *buffer, int width, int height);
    void drawOnce();
    void glDraw();

private:
    typedef struct
    {
        // Attribute locations
        GLint  positionLoc;
        GLint  texCoordLoc;

        // Sampler locations(Y,UV)
        GLint frontYLoc;
        GLint frontUVLoc;
        GLint rearYLoc;
        GLint rearUVLoc;
        GLint leftYLoc;
        GLint leftUVLoc;
        GLint rightYLoc;
        GLint rightUVLoc;

        // Texture handle(Y,U,V)
        GLuint frontYTexId;
        GLuint frontUVTexId;
        GLuint rearYTexId;
        GLuint rearUVTexId;
        GLuint leftYTexId;
        GLuint leftUVTexId;
        GLuint rightYTexId;
        GLuint rightUVTexId;

    } UserData;

    UserData mUserData;

    ICapture* mCapture;
    unsigned int mFocusChannelIndex;

    clock_t mLastCallTime;
};

#endif // GLSHADERYUV_H
