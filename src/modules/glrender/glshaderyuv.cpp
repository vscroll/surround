#include "glshaderyuv.h"
#include "ICapture.h"
#include "util.h"
#include <iostream>
#include <string.h>
#include "esUtil.h"
#include "ogldev_util.h"

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

int GLShaderYUV::initConfig()
{
    return 0;
}

void GLShaderYUV::initVertex()
{
    // Get the attribute locations
    mUserData.positionLoc = glGetAttribLocation(mProgramObject, "a_position");
    mUserData.texCoordLoc = glGetAttribLocation(mProgramObject, "a_texCoord");
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


    glGenTextures(1, &mUserData.frontYTexId);
    glGenTextures(1, &mUserData.frontUVTexId);
    glGenTextures(1, &mUserData.rearYTexId);
    glGenTextures(1, &mUserData.rearUVTexId);
    glGenTextures(1, &mUserData.leftYTexId);
    glGenTextures(1, &mUserData.leftUVTexId);
    glGenTextures(1, &mUserData.rightYTexId);
    glGenTextures(1, &mUserData.rightUVTexId);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_FRONT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_FRONT].info.height;
            unsigned char y0[width*height] = {0};
            unsigned char uv0[width*height] = {0};
            Util::yuyv_to_yuv(width, height, buffer, y0, uv0, uv0 + width*height/2);
            loadTexture(mUserData.frontYTexId, y0, width, height);
            loadTexture(mUserData.frontUVTexId, uv0, width, height);
            checkGlError("initTexture: load front image texture");

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_REAR].data);
            width = surroundImage->frame[VIDEO_CHANNEL_REAR].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_REAR].info.height;
            unsigned char y1[width*height] = {0};
            unsigned char uv1[width*height] = {0};
            Util::yuyv_to_yuv(width, height, buffer, y1, uv1, uv1 + width*height/2);
            loadTexture(mUserData.rearYTexId, y1, width, height);
            loadTexture(mUserData.rearUVTexId, uv1, width, height);
            checkGlError("initTexture: load rear image texture");  

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_LEFT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_LEFT].info.height;
            unsigned char y2[width*height] = {0};
            unsigned char uv2[width*height] = {0};
            Util::yuyv_to_yuv(width, height, buffer, y2, uv2, uv2 + width*height/2);
            loadTexture(mUserData.leftYTexId, y2, width, height);
            loadTexture(mUserData.leftUVTexId, uv2, width, height);
            checkGlError("initTexture: load left image texture");

            buffer = (unsigned char*)(surroundImage->frame[VIDEO_CHANNEL_RIGHT].data);
            width = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.width;
            height = surroundImage->frame[VIDEO_CHANNEL_RIGHT].info.height;
            unsigned char y3[width*height] = {0};
            unsigned char uv3[width*height] = {0};
            Util::yuyv_to_yuv(width, height, buffer, y3, uv3, uv3 + width*height/2);
            loadTexture(mUserData.rightYTexId, y3, width, height);
            loadTexture(mUserData.rightUVTexId, uv3, width, height);
            checkGlError("initTexture: load right image texture");
        }

        delete surroundImage;
        surroundImage = NULL;
    }
#if 0
    surround_image_t* sideImage = mCapture->popOneFrame4FocusSource();
    mFocusChannelIndex = mCapture->getFocusChannelIndex();
    if (NULL != sideImage)
    {
        unsigned char* buffer = (unsigned char*)(sideImage->data);
        int width = sideImage->info.width;
        int height = sideImage->info.height;
        unsigned char y[width*height] = {0};
        unsigned char uv[width*height] = {0};
        Util::yuyv_to_yuv(width, height, buffer, y, uv, uv + width*height/2);
        loadTexture(mUserData.focusVideoTexId[0], y, width, height);
        loadTexture(mUserData.focusVideoTexId[1], uv, width, height);
        checkGlError("initTexture: load focus image texture");

        delete sideImage;
        sideImage = NULL;
    }
#endif

    glDraw();

#if DEBUG_STITCH
    clock_t start2 = clock();
#endif

#if DEBUG_STITCH

    std::cout << "GLShaderYUV::drawOnce"
            << ", elapsed to last time:" << elapsed_to_last
            << ", elapsed to capture:" << (double)elapsed/1000
            << ", render:" << (double)(start2-start1)/CLOCKS_PER_SEC
            << std::endl;
#endif

}

void GLShaderYUV::glDraw()
{
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
    static GLfloat coordVertices[] = {  
        0.0f, 0.9566f,  
        0.6875f, 0.9566f,  
        0.0f, 0.0f,  
        0.6875f, 0.0f,   
    };
#endif

    GLushort indices[] = { 0, 1, 2, 1, 2, 3 };
      
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

    //Front
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mUserData.frontYTexId);
    glUniform1i(mUserData.frontYLoc, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mUserData.frontUVTexId);
    glUniform1i(mUserData.frontUVLoc, 1);

    //Rear
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mUserData.rearYTexId);
    glUniform1i(mUserData.rearYLoc, 2);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mUserData.rearUVTexId);
    glUniform1i(mUserData.rearUVLoc, 3);

    //Left
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, mUserData.leftYTexId);
    glUniform1i(mUserData.leftYLoc, 4);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, mUserData.leftUVTexId);
    glUniform1i(mUserData.leftUVLoc, 5);

    //right
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, mUserData.rightYTexId);
    glUniform1i(mUserData.rightYLoc, 6);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, mUserData.rightUVTexId);
    glUniform1i(mUserData.rightUVLoc, 7);

    glDrawElements ( GL_TRIANGLES, sizeof(indices)/sizeof(GLushort), GL_UNSIGNED_SHORT, indices );

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

    GLShader::shutdown();
}

GLboolean GLShaderYUV::loadTexture(GLuint textureId, unsigned char *buffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return TRUE;
}

