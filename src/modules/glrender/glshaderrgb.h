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
    virtual void initVertex();
    virtual void initTexture();
    virtual void draw();
    virtual void shutdown();

    virtual void updateFocusChannel();
    virtual void updatePanoramaView();

private:
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

        GLint lutLoc;

        // Texture handle
        GLuint frontTexId;
        GLuint rearTexId;
        GLuint leftTexId;
        GLuint rightTexId;

        GLuint lutTexId;

    } UserData;

    UserData mUserData;

    ICapture* mCapture;

    bool mUpdateLut;

    clock_t mLastCallTime;
};

#endif // GLSHADERRGB_H
