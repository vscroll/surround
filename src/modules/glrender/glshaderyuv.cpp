#include "glshaderyuv.h"
#include "ICapture.h"
#include "util.h"
#include <iostream>

#define TEST 0

#if TEST
extern unsigned char Front_uyvy_352x288[];
#endif

static const char* gVideoSamplerVar[][GLShaderYUV::YUV_CHN_NUM] = {
    "s_frontY", "s_frontU", "s_frontV",
    "s_rearY", "s_rearU", "s_rearV",
    "s_leftY", "s_leftU", "s_leftV",
    "s_rightY", "s_rightU", "s_rightV"};

static const char* gFocusVideoSamplerVar[GLShaderYUV::YUV_CHN_NUM] = {
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

GLShaderYUV::GLShaderYUV(ICapture* capture)
{
    mCapture = capture;

    mLastCallTime = 0;
}

GLShaderYUV::~GLShaderYUV()
{

}

const char* GLShaderYUV::getVertShader()
{
    return gVShaderStr;
}

const char* GLShaderYUV::getFragShader()
{
    return gFShaderStr;
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
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        for (int j = 0; j < YUV_CHN_NUM; ++j)
        {
            mUserData.videoSamplerLoc[i][j] = glGetUniformLocation(mProgramObject, gVideoSamplerVar[i][j]);
        }
    }

    for (int j = 0; j < YUV_CHN_NUM; ++j)
    {
        mUserData.focusVideoSamplerLoc[j] = glGetUniformLocation(mProgramObject, gFocusVideoSamplerVar[j]);
    }

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        for (int j = 0; j < YUV_CHN_NUM; ++j)
        {
            glGenTextures(1, &mUserData.videoTexId[i][j]);
        }
    }


    for (int j = 0; j < YUV_CHN_NUM; ++j)
    {
        glGenTextures(1, &mUserData.focusVideoTexId[j]);
    }

    checkGlError("initTexture");

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void GLShaderYUV::draw()
{
    while (true)
    {
#if TEST
        unsigned char* buffer = Front_uyvy_352x288;
        int width = 352;
        int height = 288;
        unsigned char y[width*height] = {0};
        unsigned char u[width/2*height] = {0};
        unsigned char v[width/2*height] = {0};
        Util::uyvy_to_yuv(width, height, buffer, y, u, v);
        loadTexture(mUserData.videoTexId[VIDEO_CHANNEL_FRONT][0], y, width, height);
        loadTexture(mUserData.videoTexId[VIDEO_CHANNEL_FRONT][1], u, width/2, height);
        loadTexture(mUserData.videoTexId[VIDEO_CHANNEL_FRONT][2], v, width/2, height);

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

            for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
            {
                buffer = (unsigned char*)(surroundImage->frame[i].data);
                width = surroundImage->frame[i].info.width;
                height = surroundImage->frame[i].info.height;
                unsigned char y[width*height] = {0};
                unsigned char u[width/2*height] = {0};
                unsigned char v[width/2*height] = {0};
                Util::uyvy_to_yuv(width, height, buffer, y, u, v);
                loadTexture(mUserData.videoTexId[i][0], y, width, height);
                loadTexture(mUserData.videoTexId[i][1], u, width/2, height);
                loadTexture(mUserData.videoTexId[i][2], v, width/2, height);
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
        unsigned char y[width*height] = {0};
        unsigned char u[width/2*height] = {0};
        unsigned char v[width/2*height] = {0};
        Util::uyvy_to_yuv(width, height, buffer, y, u, v);
        loadTexture(mUserData.focusVideoTexId[0], y, width, height);
        loadTexture(mUserData.focusVideoTexId[1], u, width/2, height);
        loadTexture(mUserData.focusVideoTexId[2], v, width/2, height);

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

void GLShaderYUV::glDraw()
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
    glViewport(0, 0, mESContext.width, mESContext.height);
   
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
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[0][0]);
    glUniform1i(mUserData.videoSamplerLoc[0][0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[0][1]);
    glUniform1i(mUserData.videoSamplerLoc[0][1], 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[0][2]);
    glUniform1i(mUserData.videoSamplerLoc[0][2], 2);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    eglSwapBuffers(mESContext.eglDisplay, mESContext.eglSurface);
}

void GLShaderYUV::shutdown()
{
    // Delete texture object
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        for (int j = 0; j < YUV_CHN_NUM; ++j)
        {
            glDeleteTextures(1, &mUserData.videoTexId[i][j]);
        }
    }

    for (int j = 0; j < YUV_CHN_NUM; ++j)
    {
        glDeleteTextures(1, &mUserData.focusVideoTexId[j]);
    }

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

    checkGlError("loadTexture");

    return TRUE;
}

