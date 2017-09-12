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

GLShader::GLShader(ESContext* context, const std::string programBinaryFile)
{
    mESContext = context;
    mProgramBinaryFile.assign(programBinaryFile);
    mProgramObject = -1;
}

GLShader::~GLShader()
{

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

void GLShader::checkGlError(const char* op)   
{  
    GLint error;  
    for (error = glGetError(); error; error = glGetError())   
    {  
        std::cout << "error::after " << op << "() glError (0x" << error << ")"
            << std::endl;
    }  
}

