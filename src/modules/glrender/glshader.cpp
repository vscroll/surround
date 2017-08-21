#include "glshader.h"
#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "util.h"
#include "IGLRender.h"
#include "ICapture.h"

GLShader::GLShader(ESContext* context)
{
    mESContext = context;
    mProgramObject = -1;
}

GLShader::~GLShader()
{

}

int GLShader::initProgram()
{
    // Load the shaders and get a linked program object
    mProgramObject = esLoadProgram(getVertShader(), getFragShader());

    // Use the program object
    glUseProgram (mProgramObject);

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

