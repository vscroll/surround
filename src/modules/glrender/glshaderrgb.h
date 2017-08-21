#ifndef GLSHADERRGB_H
#define GLSHADERRGB_H

#include <time.h>
#include "common.h"
#include "glshader.h"

class ICapture;
class GLShaderRGB: public GLShader
{
public:
    GLShaderRGB(ESContext* context, ICapture* capture);
    virtual ~GLShaderRGB();

    virtual const char* getVertShader();
    virtual const char* getFragShader();
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

        // Sampler location
        GLint frontLoc;
        GLint rearLoc;
        GLint leftLoc;
        GLint rightLoc;
        GLint focusLoc;

        // Texture handle
        GLuint frontTexId;
        GLuint rearTexId;
        GLuint leftTexId;
        GLuint rightTexId;
        GLuint focusTexId;

    } UserData;

    UserData mUserData;

    ICapture* mCapture;
    unsigned int mFocusChannelIndex;

    clock_t mLastCallTime;
};

#endif // GLSHADERRGB_H
