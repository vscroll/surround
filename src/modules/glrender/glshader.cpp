#include "glshader.h"
#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "util.h"
#include "IGLRender.h"
#include "ICapture.h"
#include <GLES2/gl2ext.h>
#include <linux/videodev2.h>

GLShader::GLShader(ESContext* context, const std::string programBinaryFile)
{
    mESContext = context;
    mProgramBinaryFile.assign(programBinaryFile);
    mProgramObject = -1;

    mFocusChannelIndex = VIDEO_CHANNEL_FRONT;
    mPanoramaView = PANORAMA_VIEW_BIRD;

}

GLShader::~GLShader()
{

}

int GLShader::initConfig()
{
    loadLut(mPanoramaView);
    return 0;
}

void GLShader::loadLut(int index)
{
    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return;
    }

    cv::Mat lookupTabHor;
    cv::Mat lookupTabVer;
    cv::Mat mask;

    char algoCfgPathName[1024] = {0};
    sprintf(algoCfgPathName, "%s/calibration/Lut_ChannelY_View_%d.xml", procPath, index);
    cv::FileStorage fs(algoCfgPathName, cv::FileStorage::READ);
    fs["Map_1"] >> lookupTabHor;
    fs["Map_2"] >> lookupTabVer;
    fs["Mask"] >> mask;
    fs.release();

    std::cout << "GLShader::initConfig:"
            << algoCfgPathName
            << std::endl;
    std::cout << "GLShader::initConfig"
            << ", lookupTabHor:" << lookupTabHor.cols << "x" << lookupTabHor.rows << " type:" << lookupTabHor.type()
            << std::endl;
    std::cout << "GLShader::initConfig"
            << ", lookupTabVer:" << lookupTabVer.cols << "x" << lookupTabVer.rows << " type:" << lookupTabVer.type()
            << std::endl;
    std::cout << "GLShader::initConfig"
            << ", mask:" << mask.cols << "x" << mask.rows << " type:" << mask.type()
            << std::endl;

    if (!mLutAll.empty())
    {
        mLutAll.release();
    }

    mLutAll.create(lookupTabHor.rows, lookupTabHor.cols, CV_32FC3);
    for (int i = 0; i < mLutAll.rows; i++)
    {
        for (int j = 0; j < mLutAll.cols; j++)
        {
            mLutAll.at<float>(i, j) = lookupTabHor.ptr<float>(i)[j];
            mLutAll.at<float>(i, j+1) = lookupTabVer.ptr<float>(i)[j];
            mLutAll.at<float>(i, j+2) = mask.ptr<float>(i)[j];
        }
    }

    lookupTabHor.release();
    lookupTabVer.release();
    mask.release();

    std::cout << "GLShader::initConfig"
            << ", mLutAll:" << mLutAll.cols << "x" << mLutAll.rows << " type:" << mLutAll.type()
            << std::endl;
}

GLuint GLShader::LoadProgram(unsigned char *buf, int length)
{
    const GLuint program = glCreateProgram();
    try
    {
        glProgramBinary(program, GL_PROGRAM_BINARY_VIV, buf, length);
        return program;
    }
    catch (const std::exception&)
    {
        if (program != 0)
        {
            glDeleteProgram(program);
        }
        return -1;
    }

    return program;
}

int GLShader::initProgram()
{
    // Load the shaders and get a linked program object
    if (!mProgramBinaryFile.empty()
        && Util::exists(mProgramBinaryFile.c_str()))
    {
        std::vector<unsigned char> byteArray;
        Util::readAllBytes(byteArray, mProgramBinaryFile.c_str());
        mProgramObject = LoadProgram(byteArray.data(), byteArray.size());
        std::cout << "initProgram load program:" << mProgramBinaryFile << " size:" << byteArray.size() << std::endl;
    }
    else
    {
        mProgramObject = esLoadProgram(getVertShader(), getFragShader());
#if 0
        if (mProgramObject > 0)
        {
            GLsizei length = 0;
            GLenum binaryFormat;
            std::vector<unsigned char> byteArray;
            byteArray.resize(10*1024*1024);
            glGetProgramBinary(mProgramObject, byteArray.size(), &length, &binaryFormat, byteArray.data());
            Util::writeAllBytes(mProgramBinaryFile.c_str(), byteArray, length);
            std::cout << "initProgram::write program, length:" << length << " format:" << binaryFormat << std::endl;
        }
#endif
    }

    if ( mProgramObject <= 0)
    {
        std::cout << "initProgram::error " << std::endl;
        return -1;
    }

    // Use the program object
    glUseProgram (mProgramObject);

    std::cout << "initProgram::finish " << std::endl;

    return 0;
}

void GLShader::shutdown()
{
    // Delete program object
    glDeleteProgram(mProgramObject);
}

void GLShader::getTexImageParam(unsigned int v4l2Pixfmt, GLenum* internalFormat, GLenum* format, GLenum* type)
{
    switch (v4l2Pixfmt)
    {
        case V4L2_PIX_FMT_RGB565:
        {
            *internalFormat = GL_RGB;
            *format = GL_RGB;
            *type = GL_UNSIGNED_SHORT_5_6_5;
            break;
        }
        case V4L2_PIX_FMT_RGB32:
        case V4L2_PIX_FMT_BGR32:
        {
            *internalFormat = GL_RGBA;
            *format = GL_RGBA;
            *type = GL_UNSIGNED_BYTE;
            break;
        }
        case V4L2_PIX_FMT_RGB24:
        case V4L2_PIX_FMT_BGR24:
        {
            *internalFormat = GL_RGB;
            *format = GL_RGB;
            *type = GL_UNSIGNED_BYTE;
            break;
        }
        default:
        {
            *internalFormat = GL_RGB;
            *format = GL_RGB;
            *type = GL_UNSIGNED_BYTE;
            break;
        }
     }
}

void GLShader::checkGlError(const char* op)   
{  
    GLint error;  
    for (error = glGetError(); error; error = glGetError())   
    {  
        std::cout << "error::after " << op << "() glError (0x" << error << ")"
            << std::endl;
    }  
}

