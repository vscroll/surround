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
    GLShaderYUV(ICapture* capture);
    virtual ~GLShaderYUV();

    virtual const char* getVertShader();
    virtual const char* getFragShader();
    virtual int initConfig();
    virtual void initVertex();
    virtual void initTexture();
    virtual void draw();
    virtual void shutdown();

public:
    static const unsigned int YUV_CHN_NUM = 3;

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

        // Sampler locations(Y,U,V)
        GLint videoSamplerLoc[VIDEO_CHANNEL_SIZE][YUV_CHN_NUM];
        GLint focusVideoSamplerLoc[YUV_CHN_NUM];

        // Texture handle(Y,U,V)
        GLuint videoTexId[VIDEO_CHANNEL_SIZE][YUV_CHN_NUM];
        GLuint focusVideoTexId[YUV_CHN_NUM];

    } UserData;

    UserData mUserData;

    ICapture* mCapture;
    unsigned int mFocusChannelIndex;

    cv::Mat mLookupTab[VIDEO_CHANNEL_SIZE];
    cv::Mat mMask;
    cv::Mat mWeight;

    clock_t mLastCallTime;
};

#endif // GLSHADERYUV_H
