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
        GLint lutHorSamplerLoc;
        GLint lutVerSamplerLoc;
        GLint lutMaskSamplerLoc;

        // Texture handle(Y,U,V)
        GLuint videoTexId[VIDEO_CHANNEL_SIZE][YUV_CHN_NUM];
        GLuint focusVideoTexId[YUV_CHN_NUM];
        GLuint lutHorTexId;
        GLuint lutVerTexId;
        GLuint lutMaskTexId;

    } UserData;

    UserData mUserData;

    ICapture* mCapture;
    unsigned int mFocusChannelIndex;

    cv::Mat mLookupTabHor;
    cv::Mat mLookupTabVer;
    cv::Mat mMask;

    clock_t mLastCallTime;
};

#endif // GLSHADERYUV_H
