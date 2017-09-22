#ifndef GLSHADERYUV_H
#define GLSHADERYUV_H

#include <time.h>
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

        // Sampler locations(Y,UV)
        GLint frontYLoc;
        GLint frontUVLoc;
        GLint rearYLoc;
        GLint rearUVLoc;
        GLint leftYLoc;
        GLint leftUVLoc;
        GLint rightYLoc;
        GLint rightUVLoc;

        GLint lutLoc;

        // Texture handle(Y,U,V)
        GLuint frontYTexId;
        GLuint frontUVTexId;
        GLuint rearYTexId;
        GLuint rearUVTexId;
        GLuint leftYTexId;
        GLuint leftUVTexId;
        GLuint rightYTexId;
        GLuint rightUVTexId;

        GLuint lutTexId;

    } UserData;

    UserData mUserData;

    ICapture* mCapture;

    clock_t mLastCallTime;
};

#endif // GLSHADERYUV_H
