#ifndef GLSHADERRGB_H
#define GLSHADERRGB_H

#include <time.h>
#include "common.h"
#include "glshader.h"

class ICapture;
class GLShaderRGB: public GLShader
{
public:
    GLShaderRGB(ESContext* context, const std::string programBinaryFile, ICapture* capture);
    virtual ~GLShaderRGB();

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

        // Sampler location
        GLint frontLoc;
        GLint rearLoc;
        GLint leftLoc;
        GLint rightLoc;

        GLint maskLoc;
        GLint lutHorLoc;
        GLint lutVerLoc;

        // Texture handle
        GLuint frontTexId;
        GLuint rearTexId;
        GLuint leftTexId;
        GLuint rightTexId;

        GLuint maskTexId;
        GLuint lutHorTexId;
        GLuint lutVerTexId;

    } UserData;

    UserData mUserData;

    ICapture* mCapture;
    unsigned int mFocusChannelIndex;

    clock_t mLastCallTime;
};

#endif // GLSHADERRGB_H
