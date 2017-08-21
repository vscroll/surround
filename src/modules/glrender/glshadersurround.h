#ifndef GLSHADERSURROUND_H
#define GLSHADERSURROUND_H

#include <time.h>
#include "glshader.h"

class GLShaderSurround: public GLShader
{
public:
    GLShaderSurround(ESContext* context);
    virtual ~GLShaderSurround();

    virtual const char* getVertShader();
    virtual const char* getFragShader();
    virtual int initConfig();
    virtual void initVertex();
    virtual void initTexture();
    virtual void draw();
    virtual void shutdown();

public:

private:
    GLboolean loadTexture(GLuint textureId, unsigned char *buffer, int width, int height);
    void drawOnce();
    void glUpdate();
    void glDraw();

private:
    typedef struct
    {
        GLint positionHandle;
        GLint normalHandle;
        GLint textureCoordHandle;
		
        GLint uMVPMatrixHandle;
        GLint uMMatrixHandle;
        GLint uLightLocationHandle;

        GLint uTextureHandle;

        GLuint textureId;

        // Rotation angle
        GLfloat   angle;

        // MVP matrix
        ESMatrix mvpMatrix;
        ESMatrix modelMatrix;

        // Vertex daata
        GLfloat *vertices;
        GLuint *indices;
        int numIndices;
    } UserData;

    UserData mUserData;

    clock_t mLastCallTime;
};

#endif // GLSHADERSURROUND_H
