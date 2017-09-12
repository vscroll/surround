#include "glshaderrgb.h"
#include "ICapture.h"
#include "util.h"
#include "esUtil.h"
#include <iostream>
#include <string.h>
#include "ogldev_util.h"

#if 0
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
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_front;                          \n"
      "uniform sampler2D s_rear;                           \n"
      "uniform sampler2D s_left;                           \n"
      "uniform sampler2D s_right;                          \n"
      "uniform sampler2D s_focus;                          \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = texture2D( s_front, v_texCoord );  \n"
      "}                                                   \n";   

#else
static const char gFShaderPrefix[] =
      "#version 300 es                                     \n"
      "precision mediump float;                            \n";

static std::string gVShaderStr;
static std::string gVShaderHeader;
static std::string gFShaderConst;
static std::string gFShaderStr;
#endif

GLShaderRGB::GLShaderRGB(ESContext* context, const std::string programBinaryFile, ICapture* capture)
:GLShader(context, programBinaryFile)
{
    mCapture = capture;

    mLastCallTime = 0;
}

GLShaderRGB::~GLShaderRGB()
{

}

const char* GLShaderRGB::getVertShader()
{
    ReadFile("panorama_rgb.vert", gVShaderStr);
    std::cout << gVShaderStr << std::endl;
    return gVShaderStr.c_str();
}

const char* GLShaderRGB::getFragShader()
{
#if 0
    ReadFile("panorama_rgb_frag.header", gVShaderHeader);
    ReadFile("panorama_rgb_hor.lut", gFShaderConst);
    ReadFile("panorama_rgb.frag", gFShaderStr);
    std::string str = gVShaderHeader;
    str += gFShaderConst;
    str += gFShaderStr;
    std::cout << str << std::endl;
    return str.c_str();
#else
    ReadFile("panorama_rgb.frag", gFShaderStr);
    //std::cout << gFShaderStr << std::endl;
    return gFShaderStr.c_str();
#endif
}

int GLShaderRGB::initConfig()
{
    return 0;
}

void GLShaderRGB::initVertex()
{
    // Get the attribute locations
    mUserData.positionLoc = glGetAttribLocation(mProgramObject, "a_position");
    mUserData.texCoordLoc = glGetAttribLocation(mProgramObject, "a_texCoord");
}

void GLShaderRGB::initTexture()
{
    // Get the sampler location
    mUserData.frontLoc = glGetUniformLocation(mProgramObject, "s_front");
    mUserData.rearLoc = glGetUniformLocation(mProgramObject, "s_rear");
    mUserData.leftLoc = glGetUniformLocation(mProgramObject, "s_left");
    mUserData.rightLoc = glGetUniformLocation(mProgramObject, "s_right");
    mUserData.focusLoc = glGetUniformLocation(mProgramObject, "s_focus");

    glGenTextures(1, &mUserData.frontTexId);
    glGenTextures(1, &mUserData.rearTexId);
    glGenTextures(1, &mUserData.leftTexId);
    glGenTextures(1, &mUserData.rightTexId);
    glGenTextures(1, &mUserData.focusTexId);

    checkGlError("initTexture");

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void GLShaderRGB::draw()
{
    while (true)
    {
        drawOnce();
    }
}

void GLShaderRGB::drawOnce()
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

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_FRONT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.height;
            unsigned char front[width*height*3] = {0};
            Util::yuyv_to_rgb24(width, height, buffer, front);
            loadTexture(mUserData.frontTexId, front, width, height);

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_REAR].data);
            width = surroundImage->frame[VIDEO_CHANNEL_REAR].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_REAR].info.height;
            unsigned char rear[width*height*3] = {0};
            Util::yuyv_to_rgb24(width, height, buffer, rear);
            loadTexture(mUserData.rearTexId, rear, width, height);

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_LEFT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.height;
            unsigned char left[width*height*3] = {0};
            Util::yuyv_to_rgb24(width, height, buffer, left);
            loadTexture(mUserData.leftTexId, left, width, height);

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_RIGHT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.height;
            unsigned char right[width*height*3] = {0};
            Util::yuyv_to_rgb24(width, height, buffer, right);
            loadTexture(mUserData.rightTexId, right, width, height);
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
        unsigned char side[width*height*3] = {0};
        Util::yuyv_to_rgb24(width, height, buffer, side);
        loadTexture(mUserData.focusTexId, side, width, height);

        delete sideImage;
        sideImage = NULL;
    }

    glDraw();

#if DEBUG_STITCH
    clock_t start2 = clock();
#endif

#if DEBUG_STITCH

    std::cout << "GLRenderWorker::run"
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)elapsed/1000
            << ", render:" << (double)(start2-start1)/CLOCKS_PER_SEC
            << std::endl;
#endif

}

void GLShaderRGB::glDraw()
{
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
    glViewport(0, 0, mESContext->width, mESContext->height);
   
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Load the vertex position
    glVertexAttribPointer ( mUserData.positionLoc, 2, GL_FLOAT, 
                           GL_FALSE, 2 * sizeof(GLfloat), squareVertices );
    // Load the texture coordinate
    glVertexAttribPointer ( mUserData.texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 2 * sizeof(GLfloat), coordVertices );

    glEnableVertexAttribArray(mUserData.positionLoc);
    glEnableVertexAttribArray(mUserData.texCoordLoc);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mUserData.frontTexId);
    glUniform1i(mUserData.frontLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mUserData.rearTexId);
    glUniform1i(mUserData.rearLoc, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mUserData.leftTexId);
    glUniform1i(mUserData.leftLoc, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mUserData.rightTexId);
    glUniform1i(mUserData.rightLoc, 3);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    eglSwapBuffers(mESContext->eglDisplay, mESContext->eglSurface);
}

void GLShaderRGB::shutdown()
{
    // Delete texture object

    GLShader::shutdown();
}

GLboolean GLShaderRGB::loadTexture(GLuint textureId, unsigned char *buffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D ( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    checkGlError("loadTexture");

    return TRUE;
}

