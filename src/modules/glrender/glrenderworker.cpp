#include "glrenderworker.h"
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
//#include <linux/ipu.h>
#include "util.h"
#include "IGLRender.h"
#include "ICapture.h"


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
            // Load the textures
            mUserData.frontTexId = loadTexture((unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_FRONT].data),
                surroundImage->frame[VIDEO_CHANNEL_FRONT].info.width, surroundImage->frame[VIDEO_CHANNEL_FRONT].info.height);
            mUserData.rearTexId = loadTexture((unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_REAR].data),
                surroundImage->frame[VIDEO_CHANNEL_REAR].info.width, surroundImage->frame[VIDEO_CHANNEL_REAR].info.height);
            mUserData.leftTexId = loadTexture((unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_LEFT].data),
                surroundImage->frame[VIDEO_CHANNEL_LEFT].info.width, surroundImage->frame[VIDEO_CHANNEL_LEFT].info.height);
            mUserData.rightTexId = loadTexture((unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_RIGHT].data),
                surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.width, surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.height);
        }

        delete surroundImage;
        surroundImage = NULL;
    }

    surround_image_t* sideImage = mCapture->popOneFrame4FocusSource();
    mFocusChannelIndex = mCapture->getFocusChannelIndex();
    if (NULL != sideImage)
    {
        mUserData.focusTexId = loadTexture((unsigned char*)(sideImage->data), sideImage->info.width, sideImage->info.height);
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

    if (!esCreateWindow(&mESContext, "MultiTexture", 320, 240, ES_WINDOW_RGB))
    {
        std::cout << "GLInit::esCreateWindow failed"
                << std::endl;
        return -1;
    }

    const char vShaderStr[] =  
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
    const char fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_frontMap;                       \n"
      "uniform sampler2D s_rearMap;                        \n"
      "uniform sampler2D s_leftMap;                        \n"
      "uniform sampler2D s_rightMap;                       \n"
      "uniform sampler2D s_focusMap;                       \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  vec4 frontColor;                                  \n"
      "  vec4 rearColor;                                   \n"
      "  vec4 leftColor;                                   \n"
      "  vec4 rightColor;                                  \n"
      "  vec4 focusColor;                                  \n"
      "                                                    \n"
      "  frontColor = texture2D( s_frontMap, v_texCoord ); \n"
      "  rearColor = texture2D( s_rearMap, v_texCoord );   \n"
      "  leftColor = texture2D( s_leftMap, v_texCoord );   \n"
      "  rightColor = texture2D( s_rightMap, v_texCoord ); \n"
      "  focusColor = texture2D( s_focusMap, v_texCoord ); \n"
      "  gl_FragColor = frontColor * (rearColor + 0.25);   \n"
      "}                                                   \n";

    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram(vShaderStr, fShaderStr);

    // Get the attribute locations
    userData->positionLoc = glGetAttribLocation(userData->programObject, "a_position");
    userData->texCoordLoc = glGetAttribLocation(userData->programObject, "a_texCoord");
   
    // Get the sampler location
    userData->frontLoc = glGetUniformLocation(userData->programObject, "s_frontMap");
    userData->rearLoc = glGetUniformLocation(userData->programObject, "s_rearMap");
    userData->leftLoc = glGetUniformLocation(userData->programObject, "s_leftMap");
    userData->rightLoc = glGetUniformLocation(userData->programObject, "s_rightMap");
    userData->focusLoc = glGetUniformLocation(userData->programObject, "s_focusMap");

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return 0;
}

void GLRenderWorker::GLDraw(ESContext* esContext)
{
    UserData *userData = (UserData *)esContext->userData;
    GLfloat vVertices[] = { -0.5f,  0.5f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0 
                           -0.5f, -0.5f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.5f, -0.5f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.5f,  0.5f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
      
    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);
   
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram (userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(userData->positionLoc, 3, GL_FLOAT, 
                           GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);

    glEnableVertexAttribArray(userData->positionLoc);
    glEnableVertexAttribArray(userData->texCoordLoc);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->frontTexId);
    glUniform1i(userData->frontLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, userData->rearTexId);
   
    // Set the light map sampler to texture unit 1
    glUniform1i(userData->rearLoc, 1);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    eglSwapBuffers(esContext->eglDisplay, esContext->eglSurface);
}

void GLRenderWorker::GLShutdown(ESContext* esContext)
{
   UserData *userData = (UserData *)esContext->userData;

   // Delete texture object
   glDeleteTextures(1, &userData->frontTexId);
   glDeleteTextures(1, &userData->rearTexId);
   glDeleteTextures(1, &userData->leftTexId);
   glDeleteTextures(1, &userData->rightTexId);
   glDeleteTextures(1, &userData->focusTexId);

   // Delete program object
   glDeleteProgram(userData->programObject);
}

GLuint GLRenderWorker::loadTexture(unsigned char *buffer, int width, int height)
{
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

   return texId;
}

