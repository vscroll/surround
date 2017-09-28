#include "glshaderyuv.h"
#include "ICapture.h"
#include "util.h"
#include <iostream>
#include <string.h>
#include "esUtil.h"
#include "ogldev_util.h"
#include <linux/videodev2.h>

#define DEBUG_STITCH 1

#define TEST 0

#if TEST
//this data from official G2D demo
extern unsigned char yuyv_352x288[];
#endif

static std::string gVShaderStr;
static std::string gFShaderStr;

GLShaderYUV::GLShaderYUV(ESContext* context, const std::string programBinaryFile, ICapture* capture)
:GLShader(context, programBinaryFile)
{
    mCapture = capture;

    mUpdateLut = true;

    mLastCallTime = 0;
}

GLShaderYUV::~GLShaderYUV()
{

}

const char* GLShaderYUV::getVertShader()
{
    ReadFile("panorama_yuv.vert", gVShaderStr);
    std::cout << gVShaderStr << std::endl;
    return gVShaderStr.c_str();
}

const char* GLShaderYUV::getFragShader()
{
    ReadFile("panorama_yuv.frag", gFShaderStr);
    //std::cout << gFShaderStr << std::endl;
    return gFShaderStr.c_str();
}

void GLShaderYUV::initVertex()
{
    // Get the attribute locations
    mUserData.positionLoc = glGetAttribLocation(mProgramObject, "a_position");
    mUserData.texCoordLoc = glGetAttribLocation(mProgramObject, "a_texCoord");

#if 1
    static GLfloat squareVertices[] = {  
        -1.0f, -1.0f, 
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    };
#else
    static GLfloat squareVertices[] = {
        -1, -0.95666f,
        0.0, -0.95666f,
        -1, 0.95666f,
        0.0,  0.95666f,
        1.0f, -0.95666f,
        1.0f, 0.95666f,
    };
#endif  

#if 1  
    static GLfloat coordVertices[] = {  
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };
#else
    // 600/574 = 1.045296f; 1024/704 = 1.454545f;
    static GLfloat coordVertices[] = {  
        0.0f, 1.045296f,
        1.454545f, 1.045296f,
        0.0f, 0.0f,
        1.454545f, 0.0f
    };
#endif

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

void GLShaderYUV::initTexture()
{
    // Get the sampler location
    mUserData.frontYLoc = glGetUniformLocation(mProgramObject, "s_frontY");
    mUserData.frontUVLoc = glGetUniformLocation(mProgramObject, "s_frontUV");
    mUserData.rearYLoc = glGetUniformLocation(mProgramObject, "s_rearY");
    mUserData.rearUVLoc = glGetUniformLocation(mProgramObject, "s_rearUV");
    mUserData.leftYLoc = glGetUniformLocation(mProgramObject, "s_leftY");
    mUserData.leftUVLoc = glGetUniformLocation(mProgramObject, "s_leftUV");
    mUserData.rightYLoc = glGetUniformLocation(mProgramObject, "s_rightY");
    mUserData.rightUVLoc = glGetUniformLocation(mProgramObject, "s_rightUV");

    mUserData.lutLoc = glGetUniformLocation(mProgramObject, "s_lut");


    glGenTextures(1, &mUserData.frontYTexId);
    glGenTextures(1, &mUserData.frontUVTexId);
    glGenTextures(1, &mUserData.rearYTexId);
    glGenTextures(1, &mUserData.rearUVTexId);
    glGenTextures(1, &mUserData.leftYTexId);
    glGenTextures(1, &mUserData.leftUVTexId);
    glGenTextures(1, &mUserData.rightYTexId);
    glGenTextures(1, &mUserData.rightUVTexId);

    glGenTextures(1, &mUserData.lutTexId);

    glBindTexture(GL_TEXTURE_2D, mUserData.frontYTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.frontUVTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.rearYTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.rearUVTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.leftYTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.leftUVTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.rightYTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.rightUVTexId);
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

void GLShaderYUV::updateFocusChannel()
{

}

void GLShaderYUV::updatePanoramaView()
{
    if (++mPanoramaView > PANORAMA_VIEW_NUM)
    {
        mPanoramaView = PANORAMA_VIEW_BIRD;
    }

    loadLut(mPanoramaView);

    mUpdateLut = true;
}

void GLShaderYUV::draw()
{
    while (true)
    {
#if TEST
        unsigned char* buffer = yuyv_352x288;
        int width = 352;
        int height = 288;
        unsigned char y[width*height] = {0};
        unsigned char uv[width*height] = {0};
        Util::yuyv_to_yuv(width, height, buffer, y, uv, uv+width*height/2);
        loadTexture(mUserData.frontYTexId, y, width, height);
        loadTexture(mUserData.frontUVTexId, uv, width, height);

        glDraw();
#else
        drawOnce();
#endif
    }
}

void GLShaderYUV::drawOnce()
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

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_FRONT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                unsigned char y0[width*height] = {0};
                unsigned char uv0[width*height] = {0};
                Util::yuyv_to_yuv(width, height, buffer, y0, uv0);
                glBindTexture(GL_TEXTURE_2D, mUserData.frontYTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y0);

                glBindTexture(GL_TEXTURE_2D, mUserData.frontUVTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width/2, height/2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, uv0);
                //checkGlError("initTexture: load front image texture");
            }

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_REAR].data);
            width = surroundImage->frame[VIDEO_CHANNEL_REAR].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_REAR].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_REAR].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                unsigned char y1[width*height] = {0};
                unsigned char uv1[width*height] = {0};
                Util::yuyv_to_yuv(width, height, buffer, y1, uv1);
                glBindTexture(GL_TEXTURE_2D, mUserData.rearYTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y1);

                glBindTexture(GL_TEXTURE_2D, mUserData.rearUVTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width/2, height/2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, uv1);
                //checkGlError("initTexture: load rear image texture");  
            }

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_LEFT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                unsigned char y2[width*height] = {0};
                unsigned char uv2[width*height] = {0};
                Util::yuyv_to_yuv(width, height, buffer, y2, uv2);
                glBindTexture(GL_TEXTURE_2D, mUserData.leftYTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y2);

                glBindTexture(GL_TEXTURE_2D, mUserData.leftUVTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width/2, height/2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, uv2);
                //checkGlError("initTexture: load left image texture");
            }

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_RIGHT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.height;
            pixfmt = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.pixfmt;
            if (pixfmt == V4L2_PIX_FMT_YUYV)
            {
                unsigned char y3[width*height] = {0};
                unsigned char uv3[width*height] = {0};
                Util::yuyv_to_yuv(width, height, buffer, y3, uv3);
                glBindTexture(GL_TEXTURE_2D, mUserData.rightYTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, y3);

                glBindTexture(GL_TEXTURE_2D, mUserData.rightUVTexId);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, width/2, height/2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, uv3);
                //checkGlError("initTexture: load right image texture");
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

    std::cout << "GLShaderYUV::drawOnce"
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)elapsed/1000
            << ", upload:" << (double)(start2-start1)/CLOCKS_PER_SEC
            << ", render:" << (double)(start3-start2)/CLOCKS_PER_SEC
            << ", total:" << (double)(start3-start1)/CLOCKS_PER_SEC
            << std::endl;
#endif

}

void GLShaderYUV::glDraw()
{
    glEnableVertexAttribArray(mUserData.positionLoc);
    glEnableVertexAttribArray(mUserData.texCoordLoc);

    //Front
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mUserData.frontYTexId);
    glUniform1i(mUserData.frontYLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mUserData.frontUVTexId);
    glUniform1i(mUserData.frontUVLoc, 1);

    //Rear
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mUserData.rearYTexId);
    glUniform1i(mUserData.rearYLoc, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mUserData.rearUVTexId);
    glUniform1i(mUserData.rearUVLoc, 3);

    //Left
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mUserData.leftYTexId);
    glUniform1i(mUserData.leftYLoc, 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, mUserData.leftUVTexId);
    glUniform1i(mUserData.leftUVLoc, 5);

    //right
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, mUserData.rightYTexId);
    glUniform1i(mUserData.rightYLoc, 6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, mUserData.rightUVTexId);
    glUniform1i(mUserData.rightUVLoc, 7);

    //lut
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, mUserData.lutTexId);
    if (mUpdateLut)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, PANORAMA_WIDTH, PANORAMA_HEIGHT, 0, GL_RGB, GL_FLOAT, mLutAll.data);
        mUpdateLut = false;
    }
    glUniform1i(mUserData.lutLoc, 8);

    GLushort indices[] = { 0, 1, 2, 1, 2, 3 };
    glDrawElements ( GL_TRIANGLES, sizeof(indices)/sizeof(GLushort), GL_UNSIGNED_SHORT, indices );

    glDisableVertexAttribArray(mUserData.positionLoc);
    glDisableVertexAttribArray(mUserData.texCoordLoc);

    eglSwapBuffers(mESContext->eglDisplay, mESContext->eglSurface);
}

void GLShaderYUV::shutdown()
{
    // Delete texture object
    glDeleteTextures(1, &mUserData.frontYTexId);
    glDeleteTextures(1, &mUserData.frontUVTexId);
    glDeleteTextures(1, &mUserData.rearYTexId);
    glDeleteTextures(1, &mUserData.rearUVTexId);
    glDeleteTextures(1, &mUserData.leftYTexId);
    glDeleteTextures(1, &mUserData.leftUVTexId);
    glDeleteTextures(1, &mUserData.rightYTexId);
    glDeleteTextures(1, &mUserData.rightUVTexId);

    glDeleteTextures(1, &mUserData.lutTexId);

    GLShader::shutdown();
}

