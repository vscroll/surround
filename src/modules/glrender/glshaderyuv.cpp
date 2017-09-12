#include "glshaderyuv.h"
#include "ICapture.h"
#include "util.h"
#include <iostream>
#include <string.h>
#include "esUtil.h"

#define TEST 0

#if TEST
//this data from official G2D demo
extern unsigned char yuyv_352x288[];
#endif

static const char* gVideoSamplerVar[][GLShaderYUV::YUV_CHN_NUM] = {
    "s_frontY", "s_frontU", "s_frontV",
    "s_rearY", "s_rearU", "s_rearV",
    "s_leftY", "s_leftU", "s_leftV",
    "s_rightY", "s_rightU", "s_rightV"};

static const char* gFocusVideoSamplerVar[GLShaderYUV::YUV_CHN_NUM] = {
    "s_focusY", "s_focusU", "s_focusV"};

#define STRINGIFY(A)  #A
#include "panorama2d.vert"
#include "panorama2d.frag"

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
    return gVShaderStr;
}

const char* GLShaderYUV::getFragShader()
{
    return gFShaderStr;
}

int GLShaderYUV::initConfig()
{
    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    char algoCfgPathName[1024] = {0};
    sprintf(algoCfgPathName, "%s/calibration/Lut_ChannelY_View_1.xml", procPath);
	cv::FileStorage fs(algoCfgPathName, cv::FileStorage::READ);
	fs["Map_1"] >> mLookupTabHor;
	fs["Map_2"] >> mLookupTabVer;
	fs["Mask"] >> mMask;
	fs.release();

    std::cout << "GLShaderYUV::initConfig"
            << ", lookupTabHor:" << mLookupTabHor.cols << "x" << mLookupTabHor.rows << " type:" << mLookupTabHor.type()
            << std::endl;
    std::cout << "GLShaderYUV::initConfig"
            << ", lookupTabVer:" << mLookupTabVer.cols << "x" << mLookupTabVer.rows << " type:" << mLookupTabVer.type()
            << std::endl;
    std::cout << "GLShaderYUV::initConfig"
            << ", mask:" << mMask.cols << "x" << mMask.rows << " type:" << mMask.type()
            << std::endl;

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

    checkGlError("initTexture: gene image texture");

    mUserData.lutHorSamplerLoc = glGetUniformLocation (mProgramObject, "s_lutHor");
    mUserData.lutVerSamplerLoc  = glGetUniformLocation (mProgramObject, "s_lutVer");
    mUserData.lutMaskSamplerLoc  = glGetUniformLocation (mProgramObject, "s_mask");

    glGenTextures(1, &mUserData.lutHorTexId);
    glGenTextures(1, &mUserData.lutVerTexId);
    glGenTextures(1, &mUserData.lutMaskTexId);

    checkGlError("initTexture: gene map texture");

    std::cout << "mLookupTabHor [0,0]:" << mLookupTabHor.at<float>(0*424 + 0, 0)
                           << " [423,0]:" << mLookupTabHor.at<float>(0*424 + 423, 0)
                           << std::endl;
    std::cout << "mLookupTabHor [0,599]:" << mLookupTabHor.at<float>(599*424 + 0, 0)
                           << " [423,599]:" << mLookupTabHor.at<float>(599*424 + 423, 0)
                           << std::endl;

    std::cout << "mLookupTabVer [0,0]:" << mLookupTabVer.at<float>(0*424 + 0, 0)
                           << " [423,0]:" << mLookupTabVer.at<float>(0*424 + 423, 0)
                           << std::endl;
    std::cout << "mLookupTabVer [0,599]:" << mLookupTabVer.at<float>(599*424 + 0, 0)
                           << " [423,599]:" << mLookupTabVer.at<float>(599*424 + 423, 0)
                           << std::endl;

    std::cout << "mMask [0,0]:" << mMask.at<float>(0*424 + 0, 0)
                   << " [423,0]:" << mMask.at<float>(0*424 + 423, 0)
                   << std::endl;
    std::cout << "mMask [0,599]:" << mMask.at<float>(599*424 + 0, 0)
                   << " [423,599]:" << mMask.at<float>(599*424 + 423, 0)
                   << std::endl;

    unsigned char hor[1024*600] = {};
    for(int i = 0; i < 600; i++)
    {
        for(int j = 0; j < 1024; j++)
        {
            if (i < 600 && j < 424)
            {
                hor[i*1024+j] = mLookupTabHor.at<float>(i*424 + j, 0);
            }
            else
            {
                hor[i*1024+j] = 0.0;
            }
        }
    }

    unsigned char ver[1024*600] = {};
    for(int i = 0; i < 600; i++)
    {
        for(int j = 0; j < 1024; j++)
        {
            if (i < 600 && j < 424)
            {
                ver[i*1024+j] = mLookupTabVer.at<float>(i*424 + j, 0);
            }
            else
            {
                ver[i*1024+j] = 0.0;
            }
        }
    }

    unsigned char mask[1024*600] = {};
    for(int i = 0; i < 600; i++)
    {
        for(int j = 0; j < 1024; j++)
        {
            if (i < 600 && j < 424)
            {
                mask[i*1024+j] = mMask.at<float>(i*424 + j, 0);
            }
            else
            {
                mask[i*1024+j] = 0.0;
            }
        }
    }

    std::cout << "hor [0,0]:" << hor[0*424 + 0]
                    << " [423,0]:" << hor[0*424 + 423]
                    << " [1023,0]:" << hor[0*424 + 1023]
                    << std::endl;
    std::cout << "hor [0,599]:" << hor[599*424 + 0]
                    << " [423,599]:" << hor[599*424 + 423]
                    << " [1023,599]:" << hor[599*424 + 1023]
                    << std::endl;

    std::cout << "ver [0,0]:" << ver[0*424 + 0]
                    << " [423,0]:" << ver[0*424 + 423]
                    << " [1023,0]:" << ver[0*424 + 1023]
                    << std::endl;
    std::cout << "ver [0,599]:" << ver[599*424 + 0]
                    << " [423,599]:" << ver[599*424 + 423]
                    << " [1023,599]:" << ver[599*424 + 1023]
                    << std::endl;

    std::cout << "mask [0,0]:" << mask[0*424 + 0]
                    << " [423,0]:" << mask[0*424 + 423]
                    << " [1023,0]:" << mask[0*424 + 1023]
                    << std::endl;
    std::cout << "mask [0,599]:" << mask[599*424 + 0]
                    << " [423,599]:" << mask[599*424 + 423]
                    << " [1023,599]:" << mask[599*424 + 1023]
                    << std::endl;

    glBindTexture(GL_TEXTURE_2D, mUserData.lutHorTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1024, 600, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, hor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.lutVerTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1024, 600, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ver);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, mUserData.lutMaskTexId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 1024, 600, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, mask);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    checkGlError("initTexture: load map texture");

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
        unsigned char u[width/2*height] = {0};
        unsigned char v[width/2*height] = {0};
        Util::yuyv_to_yuv(width, height, buffer, y, u, v);
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
                Util::yuyv_to_yuv(width, height, buffer, y, u, v);
                loadTexture(mUserData.videoTexId[i][0], y, width, height);
                loadTexture(mUserData.videoTexId[i][1], u, width/2, height);
                loadTexture(mUserData.videoTexId[i][2], v, width/2, height);
                checkGlError("initTexture: load side image texture");
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
        Util::yuyv_to_yuv(width, height, buffer, y, u, v);
        loadTexture(mUserData.focusVideoTexId[0], y, width, height);
        loadTexture(mUserData.focusVideoTexId[1], u, width/2, height);
        loadTexture(mUserData.focusVideoTexId[2], v, width/2, height);
        checkGlError("initTexture: load focus image texture");

        delete sideImage;
        sideImage = NULL;
    }

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
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_FRONT][0]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_FRONT][0], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_FRONT][1]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_FRONT][1], 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_FRONT][2]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_FRONT][2], 2);

    //Rear
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_REAR][0]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_REAR][0], 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_REAR][1]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_REAR][1], 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_REAR][2]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_REAR][2], 5);

    //Left
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_LEFT][0]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_LEFT][0], 6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_LEFT][1]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_LEFT][1], 7);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_LEFT][2]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_LEFT][2], 8);

    //right
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_RIGHT][0]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_RIGHT][0], 9);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_RIGHT][1]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_RIGHT][1], 10);

    glActiveTexture(GL_TEXTURE11);
    glBindTexture(GL_TEXTURE_2D, mUserData.videoTexId[VIDEO_CHANNEL_RIGHT][2]);
    glUniform1i(mUserData.videoSamplerLoc[VIDEO_CHANNEL_RIGHT][2], 11);

    //focus
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, mUserData.focusVideoTexId[0]);
    glUniform1i(mUserData.focusVideoSamplerLoc[0], 12);

    glActiveTexture(GL_TEXTURE13);
    glBindTexture(GL_TEXTURE_2D, mUserData.focusVideoTexId[1]);
    glUniform1i(mUserData.focusVideoSamplerLoc[1], 13);

    glActiveTexture(GL_TEXTURE14);
    glBindTexture(GL_TEXTURE_2D, mUserData.focusVideoTexId[2]);
    glUniform1i(mUserData.focusVideoSamplerLoc[2], 14);

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, mUserData.lutHorTexId);
    glUniform1i(mUserData.lutHorSamplerLoc, 15);

    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, mUserData.lutVerTexId);
    glUniform1i(mUserData.lutVerSamplerLoc, 16);

    glActiveTexture(GL_TEXTURE17);
    glBindTexture(GL_TEXTURE_2D, mUserData.lutMaskTexId);
    glUniform1i(mUserData.lutMaskSamplerLoc, 17);

    glDrawElements ( GL_TRIANGLES, sizeof(indices)/sizeof(GLushort), GL_UNSIGNED_SHORT, indices );

    eglSwapBuffers(mESContext->eglDisplay, mESContext->eglSurface);
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

    return TRUE;
}

