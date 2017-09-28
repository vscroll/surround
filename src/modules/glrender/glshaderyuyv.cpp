#include "glshaderyuyv.h"
#include "ICapture.h"
#include "util.h"
#include <iostream>
#include <string.h>
#include "esUtil.h"
#include "ogldev_util.h"
#include <linux/videodev2.h>
#include <GLES2/gl2ext.h>

#define GL_VIV_YUY2 0x8FC2

//#define DEBUG_STITCH 1

static std::string gVShaderStr;
static std::string gFShaderStr;

GLShaderYUYV::GLShaderYUYV(ESContext* context, const std::string programBinaryFile, ICapture* capture)
:GLShader(context, programBinaryFile)
{
    mCapture = capture;

    mUpdateLut = true;

    mLastCallTime = 0;
}

GLShaderYUYV::~GLShaderYUYV()
{

}

const char* GLShaderYUYV::getVertShader()
{
    ReadFile("panorama_yuyv.vert", gVShaderStr);
    return gVShaderStr.c_str();
}

const char* GLShaderYUYV::getFragShader()
{
    ReadFile("panorama_yuyv.frag", gFShaderStr);
    return gFShaderStr.c_str();
}

void GLShaderYUYV::initVertex()
{
    // Get the attribute locations
    mUserData.positionLoc = glGetAttribLocation(mProgramObject, "a_position");
    mUserData.texCoordLoc = glGetAttribLocation(mProgramObject, "a_texCoord");

    static GLfloat squareVertices[] = {  
        -1.0f, -1.0f, 
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    };

    static GLfloat coordVertices[] = {  
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
      
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

void GLShaderYUYV::initTexture()
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
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, PANORAMA_WIDTH, PANORAMA_HEIGHT, 0, GL_RGB, GL_FLOAT, mLutAll.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void GLShaderYUYV::updateFocusChannel()
{

}

void GLShaderYUYV::updatePanoramaView()
{
    if (++mPanoramaView > PANORAMA_VIEW_NUM)
    {
        mPanoramaView = PANORAMA_VIEW_BIRD;
    }

    loadLut(mPanoramaView);

    mUpdateLut = true;
}

void GLShaderYUYV::draw()
{
    while (true)
    {
        drawOnce();
    }
}

void GLShaderYUYV::drawOnce()
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
    start1 = clock();
#endif

    long elapsed = 0;
    surround_images_t* surroundImage = mCapture->popOneFrame();
    if (NULL != surroundImage)
    {
        elapsed = Util::get_system_milliseconds() - surroundImage->timestamp;
        if (elapsed < 400)
        {
            // bind the textures
            void* buffer;
            unsigned int pAddr;
            int width;
            int height;
            int pixfmt;

            buffer = surroundImage->frame[VIDEO_CHANNEL_FRONT].data;
            pAddr = surroundImage->frame[VIDEO_CHANNEL_FRONT].pAddr;
            width = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                glBindTexture(GL_TEXTURE_2D, mUserData.frontTexId);
#if 0
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width/2, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
#else
                glTexDirectVIVMap (GL_TEXTURE_2D, width, height, GL_VIV_YUY2, &buffer, &pAddr);
                glTexDirectInvalidateVIV (GL_TEXTURE_2D);
#endif
            }

            buffer = surroundImage->frame[VIDEO_CHANNEL_REAR].data;
            pAddr = surroundImage->frame[VIDEO_CHANNEL_REAR].pAddr;
            width = surroundImage->frame[VIDEO_CHANNEL_REAR].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_REAR].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_REAR].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                glBindTexture(GL_TEXTURE_2D, mUserData.rearTexId);
#if 0
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width/2, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
#else
                glTexDirectVIVMap (GL_TEXTURE_2D, width, height, GL_VIV_YUY2, &buffer, &pAddr);
                glTexDirectInvalidateVIV (GL_TEXTURE_2D);
#endif
            }

            buffer = surroundImage->frame[VIDEO_CHANNEL_LEFT].data;
            pAddr = surroundImage->frame[VIDEO_CHANNEL_LEFT].pAddr;
            width = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                glBindTexture(GL_TEXTURE_2D, mUserData.leftTexId);
#if 0
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width/2, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
#else
                glTexDirectVIVMap (GL_TEXTURE_2D, width, height, GL_VIV_YUY2, &buffer, &pAddr);
                glTexDirectInvalidateVIV (GL_TEXTURE_2D);
#endif
            }

            buffer = surroundImage->frame[VIDEO_CHANNEL_RIGHT].data;
            pAddr = surroundImage->frame[VIDEO_CHANNEL_RIGHT].pAddr;
            width = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                glBindTexture(GL_TEXTURE_2D, mUserData.rightTexId);
#if 0
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width/2, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
#else
                glTexDirectVIVMap (GL_TEXTURE_2D, width, height, GL_VIV_YUY2, &buffer, &pAddr);
                glTexDirectInvalidateVIV (GL_TEXTURE_2D);
#endif
            }
        }

        delete surroundImage;
        surroundImage = NULL;
    }
    mUpdateLut = true;
#if DEBUG_STITCH
    clock_t start2 = clock();
#endif

    glDraw();

#if DEBUG_STITCH
    clock_t start3 = clock();
#endif

#if DEBUG_STITCH

    std::cout << "GLShaderYUYV::drawOnce"
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)elapsed/1000
            << ", upload:" << (double)(start2-start1)/CLOCKS_PER_SEC
            << ", render:" << (double)(start3-start2)/CLOCKS_PER_SEC
            << ", total:" << (double)(start3-start1)/CLOCKS_PER_SEC
            << std::endl;
#endif

}

void GLShaderYUYV::glDraw()
{
    glEnableVertexAttribArray(mUserData.positionLoc);
    glEnableVertexAttribArray(mUserData.texCoordLoc);

    //Front
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mUserData.frontTexId);
    glUniform1i(mUserData.frontLoc, 0);

    //Rear
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mUserData.rearTexId);
    glUniform1i(mUserData.rearLoc, 1);

    //Left
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mUserData.leftTexId);
    glUniform1i(mUserData.leftLoc, 2);

    //right
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mUserData.rightTexId);
    glUniform1i(mUserData.rightLoc, 3);

    //lut
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mUserData.lutTexId);
    if (mUpdateLut)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, PANORAMA_WIDTH, PANORAMA_HEIGHT, 0, GL_RGB, GL_FLOAT, mLutAll.data);
        mUpdateLut = false;
    }
    glUniform1i(mUserData.lutLoc, 4);

    GLushort indices[] = { 0, 1, 2, 1, 2, 3 };
    glDrawElements ( GL_TRIANGLES, sizeof(indices)/sizeof(GLushort), GL_UNSIGNED_SHORT, indices );

    glDisableVertexAttribArray(mUserData.positionLoc);
    glDisableVertexAttribArray(mUserData.texCoordLoc);

    eglSwapBuffers(mESContext->eglDisplay, mESContext->eglSurface);
}

void GLShaderYUYV::shutdown()
{
    // Delete texture object
    glDeleteTextures(1, &mUserData.frontTexId);
    glDeleteTextures(1, &mUserData.rearTexId);
    glDeleteTextures(1, &mUserData.leftTexId);
    glDeleteTextures(1, &mUserData.rightTexId);

    glDeleteTextures(1, &mUserData.lutTexId);

    GLShader::shutdown();
}

