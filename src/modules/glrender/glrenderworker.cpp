#include "glrenderworker.h"
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "util.h"
#include "IGLRender.h"
#include "ICapture.h"

#define TEST 0

#if TEST
extern unsigned char Front_uyvy_352x288[];
#endif

static const char* gVideoSamplerVar[][GLRenderWorker::YUV_CHN_NUM] = {
    "s_frontY", "s_frontU", "s_frontV",
    "s_rearY", "s_rearU", "s_rearV",
    "s_leftY", "s_leftU", "s_leftV",
    "s_rightY", "s_rightU", "s_rightV"};

static const char* gFocusVideoSamplerVar[GLRenderWorker::YUV_CHN_NUM] = {
    "s_focusY", "s_focusU", "s_focusV"};

static const char gVShaderStr[] =  
    "attribute vec4 a_position;     \n"
    "attribute vec2 a_texCoord;     \n"
    "varying vec2 v_texCoord;       \n"
    "void main()                    \n"
    "{                              \n"
    "   gl_Position = a_position;   \n"
    "   v_texCoord = a_texCoord;    \n"
    "}                              \n";
   
static const char gFShaderStr[] =  
    "precision mediump float;                               \n"
    "varying vec2 v_texCoord;                               \n"
    "uniform sampler2D s_frontY;                            \n"
    "uniform sampler2D s_frontU;                            \n"
    "uniform sampler2D s_frontV;                            \n"
    "uniform sampler2D s_rearY;                             \n"
    "uniform sampler2D s_rearU;                             \n"
    "uniform sampler2D s_rearV;                             \n"
    "uniform sampler2D s_leftY;                             \n"
    "uniform sampler2D s_leftU;                             \n"
    "uniform sampler2D s_leftV;                             \n"
    "uniform sampler2D s_rightY;                            \n"
    "uniform sampler2D s_rightU;                            \n"
    "uniform sampler2D s_rightV;                            \n"
    "uniform sampler2D s_focusY;                            \n"
    "uniform sampler2D s_focusU;                            \n"
    "uniform sampler2D s_focusV;                            \n"
    "void main()                                            \n"
    "{                                                      \n"
    "   mediump vec3 yuv;                                   \n"
    "   lowp vec3 rgb;                                      \n"
    "   yuv.x = texture2D(s_frontY, v_texCoord).r;          \n"
    "   yuv.y = texture2D(s_frontU, v_texCoord).r - 0.5;    \n"
    "   yuv.z = texture2D(s_frontV, v_texCoord).r - 0.5;    \n"
    "   rgb = mat3( 1,   1,   1,                            \n"
    "               0,         -0.39465,  2.03211,          \n"
    "               1.13983,   -0.58060,  0) * yuv;         \n"
    "   gl_FragColor = vec4(rgb, 1);                        \n"
    "}                                                      \n";

GLRenderWorker::GLRenderWorker()
{
    mCapture = NULL;
    
    mDisplayMode = IGLRender::DISPLAY_MODE_PANO_PLUS_FRONT;

    mPanoramaViewleft = 0;
    mPanoramaViewTop = 0;
    mPanoramaViewWidth = 0;
    mPanoramaViewHeight = 0;

    mXViewleft = 0;
    mXViewTop = 0;
    mXViewWidth = 0;
    mXViewHeight = 0;

    memset(&mScreenInfo, 0, sizeof(mScreenInfo));

    mFocusChannelIndex = VIDEO_CHANNEL_FRONT;

    mLastCallTime = 0;
}

GLRenderWorker::~GLRenderWorker()
{

}

void GLRenderWorker::setDisplayMode(unsigned int displayMode)
{
    if (displayMode >= IGLRender::DISPLAY_MODE_MIN
        && displayMode <= IGLRender::DISPLAY_MODE_MAX)
    {
        mDisplayMode = displayMode;
    }
}

void GLRenderWorker::setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{
}

void GLRenderWorker::setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height)
{

}

int GLRenderWorker::init(ICapture* capture)
{
    mCapture = capture;
    getScreenInfo(0);
    if (GLInit() < 0)
    {
        return -1;
    }

    return 0;
}

void GLRenderWorker::draw()
{
    //all the opengl es functions must be called in one thread
    while (true)
    {
#if TEST
        unsigned char* buffer = Front_uyvy_352x288;
        int width = 352;
        int height = 288;
        loadTexture(mUserData.frontYTexId, buffer, width, height);
        loadTexture(mUserData.frontUTexId, buffer + width * height, width/2, height/2);
        loadTexture(mUserData.frontVTexId, buffer + width * height * 5 / 4, width/2, height/2);

        GLDraw(&mESContext);
#else
        run();
#endif
    }
}

void GLRenderWorker::run()
{
#if DEBUG_STITCH
    clock_t start0 = clock();
    double elapsed_to_last = 0;
    if (mLastCallTime != 0)
    {
        elapsed_to_last = (double)(start0 - mLastCallTime)/CLOCKS_PER_SEC;
    }
    mLastCallTime = start0;
#endif

    if (NULL == mCapture)
    {
        return;
    }

#if DEBUG_STITCH
    clock_t start1 = clock();
#endif

    surround_images_t* surroundImage = mCapture->popOneFrame();
    if (NULL != surroundImage)
    {
        long elapsed = Util::get_system_milliseconds() - surroundImage->timestamp;
        if (elapsed < 400)
        {
            // bind the textures
            unsigned char* buffer;
            int width;
            int height;

            for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
            {
                buffer = (unsigned char*)(surroundImage->frame[i].data);
                width = surroundImage->frame[i].info.width;
                height = surroundImage->frame[i].info.height;
                loadTexture(mUserData.videoTexId[i][0], buffer, width, height);
                loadTexture(mUserData.videoTexId[i][1], buffer + width * height, width/2, height/2);
                loadTexture(mUserData.videoTexId[i][2], buffer + width * height * 5 / 4, width/2, height/2);
            }
        }

        delete surroundImage;
        surroundImage = NULL;
    }

    surround_image_t* sideImage = mCapture->popOneFrame4FocusSource();
    mFocusChannelIndex = mCapture->getFocusChannelIndex();
    if (NULL != sideImage)
    {
        unsigned char* buffer = (unsigned char*)(sideImage->data);
        int width = sideImage->info.width;
        int height = sideImage->info.height;
        loadTexture(mUserData.focusvideoTexId[0], buffer, width, height);
        loadTexture(mUserData.focusvideoTexId[1], buffer + width * height, width/2, height/2);
        loadTexture(mUserData.focusvideoTexId[2], buffer + width * height * 5 / 4, width/2, height/2);

        delete sideImage;
        sideImage = NULL;
    }

    GLDraw(&mESContext);

#if DEBUG_STITCH
    clock_t start2 = clock();
#endif

#if DEBUG_STITCH

    std::cout << "GLRenderWorker::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)elapsed/1000
            << ", render:" << (double)(start2-start1)/CLOCKS_PER_SEC
            << std::endl;
#endif
}

int GLRenderWorker::getScreenInfo(int devIndex)
{
    int fd;
    char fb[16] = {0};
    sprintf(fb, "/dev/fb%d", devIndex);
    if ((fd = open(fb, O_RDWR, 0)) < 0)
    {
        std::cout << "open failed:" << fb
                  << std::endl;
	    return -1;
    }

    /* Get variable screen info. */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &mScreenInfo) < 0)
    {
        std::cout << "FBIOGET_VSCREENINFO failed"
                  << std::endl;
    }

    std::cout << "GLRenderWorker::getScreenInfo"
              << " fb:" << fb
              << " xres_virtual:" << mScreenInfo.xres_virtual
              << " yres_virtual:" << mScreenInfo.yres_virtual
              << ", xres:" << mScreenInfo.xres
              << ", yres:" << mScreenInfo.yres
              << std::endl;

    close(fd);
    return 0;
}

int GLRenderWorker::GLInit()
{
    esInitContext(&mESContext);
    mESContext.userData = &mUserData;
    UserData *userData = (UserData *)mESContext.userData;

    if (!esCreateWindow(&mESContext, "MultiTexture", mScreenInfo.xres, mScreenInfo.yres, ES_WINDOW_RGB))
    {
        std::cout << "GLInit::esCreateWindow failed"
                << std::endl;
        return -1;
    }

    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram(gVShaderStr, gFShaderStr);

    // Use the program object
    glUseProgram (userData->programObject);

    // Get the attribute locations
    userData->positionLoc = glGetAttribLocation(userData->programObject, "a_position");
    userData->texCoordLoc = glGetAttribLocation(userData->programObject, "a_texCoord");
   
    // Get the sampler location
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        for (int j = 0; j < YUV_CHN_NUM; ++j)
        {
            userData->videoSamplerLoc[i][j] = glGetUniformLocation(userData->programObject, gVideoSamplerVar[i][j]);
        }
    }

    for (int j = 0; j < YUV_CHN_NUM; ++j)
    {
        userData->focusVideoSamplerLoc[j] = glGetUniformLocation(userData->programObject, gFocusVideoSamplerVar[j]);
    }

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        for (int j = 0; j < YUV_CHN_NUM; ++j)
        {
            glGenTextures(1, &userData->videoTexId[i][j]);
        }
    }


    for (int j = 0; j < YUV_CHN_NUM; ++j)
    {
        glGenTextures(1, &userData->focusvideoTexId[j]);
    }

    GLCheckGlError("GLInit");

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return 0;
}

void GLRenderWorker::GLDraw(ESContext* esContext)
{
    UserData *userData = (UserData *)esContext->userData;
    static GLfloat squareVertices[] = {  
        -1.0f, -1.0f,  
        1.0f, -1.0f,  
        -1.0f,  1.0f,  
        1.0f,  1.0f,  
    };  
  
    static GLfloat coordVertices[] = {  
        0.0f, 1.0f,  
        1.0f, 1.0f,  
        0.0f,  0.0f,  
        1.0f,  0.0f,  
    };
      
    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);
   
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Load the vertex position
    glVertexAttribPointer ( userData->positionLoc, 2, GL_FLOAT, 
                           GL_FALSE, 2 * sizeof(GLfloat), squareVertices );
    // Load the texture coordinate
    glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 2 * sizeof(GLfloat), coordVertices );

    glEnableVertexAttribArray(userData->positionLoc);
    glEnableVertexAttribArray(userData->texCoordLoc);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->videoTexId[0][0]);
    glUniform1i(userData->videoSamplerLoc[0][0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, userData->videoTexId[0][1]);
    glUniform1i(userData->videoSamplerLoc[0][1], 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, userData->videoTexId[0][2]);
    glUniform1i(userData->videoSamplerLoc[0][2], 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void GLRenderWorker::GLShutdown(ESContext* esContext)
{
    UserData *userData = (UserData *)esContext->userData;

    // Delete texture object
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        for (int j = 0; j < YUV_CHN_NUM; ++j)
        {
            glDeleteTextures(1, &userData->videoTexId[i][j]);
        }
    }

    for (int j = 0; j < YUV_CHN_NUM; ++j)
    {
        glDeleteTextures(1, &userData->focusvideoTexId[j]);
    }

    // Delete program object
    glDeleteProgram(userData->programObject);
}

GLboolean GLRenderWorker::loadTexture(GLuint textureId, unsigned char *buffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLCheckGlError("GLInit");

    return TRUE;
}

void GLRenderWorker::GLCheckGlError(const char* op)   
{  
    GLint error;  
    for (error = glGetError(); error; error = glGetError())   
    {  
        std::cout << "error::after " << op << "() glError (0x" << error << ")"
            << std::endl;
    }  
}

