#ifndef GLRENDERWORKER_H
#define GLRENDERWORKER_H

#include "common.h"
#include "wrap_thread.h"
#include <linux/mxcfb.h>
#include "esUtil.h"

class ICapture;
class GLRenderWorker : public WrapThread
{
public:
    GLRenderWorker();
    virtual ~GLRenderWorker();

    void setDisplayMode(unsigned int displayMode);
    void setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    void setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height);
    int init(ICapture* capture);

    void draw();
public:
    virtual void run();

public:
    static const unsigned int YUV_CHN_NUM = 3;

private:
    typedef struct
    {
        // Handle to a program object
        GLuint programObject;

        // Attribute locations
        GLint  positionLoc;
        GLint  texCoordLoc;

        // Sampler locations(Y,U,V)
        GLint videoSamplerLoc[VIDEO_CHANNEL_SIZE][YUV_CHN_NUM];
        GLint focusVideoSamplerLoc[YUV_CHN_NUM];

        // Texture handle(Y,U,V)
        GLuint videoTexId[VIDEO_CHANNEL_SIZE][YUV_CHN_NUM];
        GLuint focusvideoTexId[YUV_CHN_NUM];

    } UserData;

    int GLInit();
    void GLDraw(ESContext* esContext);
    void GLShutdown(ESContext* esContext);
    GLboolean loadTexture(GLuint textureId, unsigned char *buffer, int width, int height);
    void GLCheckGlError(const char* op);

    int getScreenInfo(int devIndex);

private:
    ICapture* mCapture;

    unsigned int mDisplayMode;

    unsigned int mPanoramaViewleft;
    unsigned int mPanoramaViewTop;
    unsigned int mPanoramaViewWidth;
    unsigned int mPanoramaViewHeight;

    unsigned int mXViewleft;
    unsigned int mXViewTop;
    unsigned int mXViewWidth;
    unsigned int mXViewHeight;

    struct fb_var_screeninfo mScreenInfo;

    unsigned int mFocusChannelIndex;

    ESContext mESContext;
    UserData mUserData;

    clock_t mLastCallTime;
};

#endif // GLRENDERWORKER_H
