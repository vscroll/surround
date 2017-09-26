#include "glshaderrgb.h"
#include "ICapture.h"
#include "util.h"
#include "esUtil.h"
#include <iostream>
#include <string.h>
#include "ogldev_util.h"

#define DEBUG_STITCH 1

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
static std::string gVShaderStr;
static std::string gFShaderStr;
#endif

GLShaderRGB::GLShaderRGB(ESContext* context, const std::string programBinaryFile, ICapture* capture)
:GLShader(context, programBinaryFile)
{
    mCapture = capture;

    mUpdateLut = true;

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
    ReadFile("panorama_rgb.frag", gFShaderStr);
    //std::cout << gFShaderStr << std::endl;
    return gFShaderStr.c_str();
}

void GLShaderRGB::initVertex()
{
    // Get the attribute locations
    mUserData.positionLoc = glGetAttribLocation(mProgramObject, "a_position");
    mUserData.texCoordLoc = glGetAttribLocation(mProgramObject, "a_texCoord");

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
}

void GLShaderRGB::initTexture()
{
    // Get the sampler location
    mUserData.frontLoc = glGetUniformLocation(mProgramObject, "s_front");
    mUserData.rearLoc = glGetUniformLocation(mProgramObject, "s_rear");
    mUserData.leftLoc = glGetUniformLocation(mProgramObject, "s_left");
    mUserData.rightLoc = glGetUniformLocation(mProgramObject, "s_right");

    mUserData.lutLoc = glGetUniformLocation(mProgramObject, "s_lut");

    glGenTextures(1, &mUserData.frontTexId);
    glGenTextures(1, &mUserData.rearTexId);
    glGenTextures(1, &mUserData.leftTexId);
    glGenTextures(1, &mUserData.rightTexId);

    glGenTextures(1, &mUserData.lutTexId);

    glBindTexture(GL_TEXTURE_2D, mUserData.frontTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.rearTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.leftTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.rightTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //lut
    glBindTexture(GL_TEXTURE_2D, mUserData.lutTexId);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, PANORAMA_WIDTH, PANORAMA_HEIGHT, 0, GL_RGB, GL_FLOAT, mLutAll->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GLShaderRGB::updateFocusChannel()
{

}

void GLShaderRGB::updatePanoramaView()
{
    if (++mPanoramaView > PANORAMA_VIEW_NUM)
    {
        mPanoramaView = PANORAMA_VIEW_BIRD;
    }

    loadLut(mPanoramaView);

    //must run in single thread
    //glBindTexture(GL_TEXTURE_2D, mUserData.lutTexId);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, PANORAMA_WIDTH, PANORAMA_HEIGHT, 0, GL_RGB, GL_FLOAT, mLutAll->data);
    mUpdateLut = true;
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
    clock_t start1 = 0;
    clock_t start2 = 0;
#endif
    long elapsed = 0;
    surround_images_t* surroundImage = mCapture->popOneFrame();
    if (NULL != surroundImage)
    {
        elapsed = Util::get_system_milliseconds() - surroundImage->timestamp;
        if (elapsed < 400)
        {
            // bind the textures
            unsigned char* buffer;
            int width;
            int height;
            int pixfmt;
            GLenum internalFormat;
            GLenum format;
            GLenum type;

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_FRONT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.pixfmt;
            //unsigned char front[width*height*3] = {0};
            //Util::yuyv_to_rgb24(width, height, buffer, front);
            glBindTexture(GL_TEXTURE_2D, mUserData.frontTexId);
            getTexImageParam(pixfmt, &internalFormat, &format, &type);
            start1 = clock();
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, buffer);
            start2 = clock();

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_REAR].data);
            width = surroundImage->frame[VIDEO_CHANNEL_REAR].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_REAR].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_REAR].info.pixfmt;
            //unsigned char rear[width*height*3] = {0};
            //Util::yuyv_to_rgb24(width, height, buffer, rear);
            glBindTexture(GL_TEXTURE_2D, mUserData.rearTexId);
            getTexImageParam(pixfmt, &internalFormat, &format, &type);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, buffer);

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_LEFT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.pixfmt;
            //unsigned char left[width*height*3] = {0};
            //Util::yuyv_to_rgb24(width, height, buffer, left);
            glBindTexture(GL_TEXTURE_2D, mUserData.leftTexId);
            getTexImageParam(pixfmt, &internalFormat, &format, &type);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, buffer);

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_RIGHT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.pixfmt;
            //unsigned char right[width*height*3] = {0};
            //Util::yuyv_to_rgb24(width, height, buffer, right);
            glBindTexture(GL_TEXTURE_2D, mUserData.rightTexId);
            getTexImageParam(pixfmt, &internalFormat, &format, &type);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, buffer);
        }

        delete surroundImage;
        surroundImage = NULL;
    }

#if DEBUG_STITCH
    clock_t start3 = clock();
#endif

    glDraw();

#if DEBUG_STITCH
    clock_t start4 = clock();
#endif

#if DEBUG_STITCH

    std::cout << "GLShaderRGB::drawOnce"
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)elapsed/1000
            << ", upload:" << (double)(start2-start1)/CLOCKS_PER_SEC
            << ", render:" << (double)(start4-start3)/CLOCKS_PER_SEC
            << ", total:" << (double)(start4-start1)/CLOCKS_PER_SEC
            << std::endl;
#endif

}

void GLShaderRGB::glDraw()
{
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

    //lut
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mUserData.lutTexId);
    if (mUpdateLut)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, PANORAMA_WIDTH, PANORAMA_HEIGHT, 0, GL_RGB, GL_FLOAT, mLutAll->data);
        mUpdateLut = false;
    }
    glUniform1i(mUserData.lutLoc, 4);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(mUserData.positionLoc);
    glDisableVertexAttribArray(mUserData.texCoordLoc);

    eglSwapBuffers(mESContext->eglDisplay, mESContext->eglSurface);
}

void GLShaderRGB::shutdown()
{
    // Delete texture object
    glDeleteTextures(1, &mUserData.frontTexId);
    glDeleteTextures(1, &mUserData.rearTexId);
    glDeleteTextures(1, &mUserData.leftTexId);
    glDeleteTextures(1, &mUserData.rightTexId);

    glDeleteTextures(1, &mUserData.lutTexId);

    GLShader::shutdown();
}

