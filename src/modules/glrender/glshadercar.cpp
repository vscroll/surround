#include "glshadercar.h"
#include "ICapture.h"
#include "util.h"
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "esUtil.h"

#define STRINGIFY(A)  #A
#include "modelcar.vert"
#include "modelcar.frag"

extern float carVerts[];
extern float carNormals[];
extern float carTexCoords[];
extern unsigned int carNumVerts;

GLShaderCar::GLShaderCar(ESContext* context)
:GLShader(context)
{
    mLastCallTime = 0;
}

GLShaderCar::~GLShaderCar()
{

}

const char* GLShaderCar::getVertShader()
{
    return gVShaderStr;
}

const char* GLShaderCar::getFragShader()
{
    return gFShaderStr;
}

int GLShaderCar::initConfig()
{
    return 0;
}

void GLShaderCar::initVertex()
{
    // Get the attribute locations
	mUserData.positionHandle = glGetAttribLocation(mProgramObject, "aPosition");
    mUserData.normalHandle = glGetAttribLocation(mProgramObject, "aNormal");
    mUserData.textureCoordHandle = glGetAttribLocation(mProgramObject, "aTextureCoord");
		
    mUserData.uMVPMatrixHandle = glGetUniformLocation(mProgramObject, "uMVPMatrix");

    mUserData.uMMatrixHandle = glGetUniformLocation(mProgramObject, "uMMatrix");
    mUserData.uLightLocationHandle = glGetUniformLocation(mProgramObject, "uLightLocation");

    mUserData.angle = 45.0f;

    checkGlError("initVertex");
}

void GLShaderCar::initTexture()
{
    mUserData.uTextureHandle = glGetUniformLocation(mProgramObject, "uTexture");

    glGenTextures(1, &mUserData.textureId);

    int width;
    int height;
    char *buffer = esLoadTGA("banana.tga", &width, &height);
    loadTexture(mUserData.textureId, (unsigned char*)buffer, width, height);
    free (buffer);

    checkGlError("initTexture");
}

void GLShaderCar::draw()
{
    drawOnce();
}

void GLShaderCar::drawOnce()
{
    glUseProgram (mProgramObject);
    glUpdate();
    glDraw();
}

void GLShaderCar::glUpdate ()
{
   ESMatrix perspective;
   float    aspect;
   
   // Compute a rotation angle based on time to rotate the cube
   //mUserData.angle += ( deltaTime * 40.0f );
   if( mUserData.angle >= 360.0f )
      mUserData.angle -= 360.0f;

   // Compute the window aspect ratio
   aspect = (GLfloat) mESContext->width / (GLfloat) mESContext->height;
   
   // Generate a perspective matrix with a 60 degree FOV
   esMatrixLoadIdentity( &perspective );
   esPerspective( &perspective, 30.0f, aspect, 1.0f, 20.0f );

   // Generate a model view matrix to rotate/translate the cube
   esMatrixLoadIdentity( &mUserData.modelMatrix );

   // Translate away from the viewer
   esTranslate( &mUserData.modelMatrix, 0.0, 0.0, -2.0 );

   // Rotate the cube
   //esRotate( &mUserData.modelMatrix, mUserData.angle, 1.0, 0.0, 1.0 );
    esRotate( &mUserData.modelMatrix, -90, 0.0, 1.0, 0.0 );
    //esRotate( &mUserData.modelMatrix, -90, 0.0, 0.0, 1.0 );
   
   // Compute the final MVP by multiplying the 
   // modevleiw and perspective matrices together
   esMatrixMultiply( &mUserData.mvpMatrix, &mUserData.modelMatrix, &perspective );
}

void GLShaderCar::glDraw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear ( GL_COLOR_BUFFER_BIT );

    //glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the viewport
    glViewport ( 0, 0, mESContext->width, mESContext->height );

    glEnable(GL_DEPTH_TEST);
    // If enabled, cull polygons based on their winding in window coordinates
    glEnable(GL_CULL_FACE);
    // Culls back face
    glCullFace(GL_BACK);

    float* verts = carVerts;
    float* normals = carNormals;
    float* texCoords = carTexCoords;
    unsigned int numVerts = carNumVerts;

    glVertexAttribPointer(mUserData.positionHandle, 3,  GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), verts);
    glVertexAttribPointer(mUserData.normalHandle, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), normals);
    glVertexAttribPointer(mUserData.textureCoordHandle, 2,  GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), texCoords);

    glEnableVertexAttribArray(mUserData.positionHandle);
    glEnableVertexAttribArray(mUserData.normalHandle);
    glEnableVertexAttribArray(mUserData.textureCoordHandle);
	
    glUniform3f(mUserData.uLightLocationHandle, 0, 0, 10);
    glUniformMatrix4fv(mUserData.uMVPMatrixHandle, 1, GL_FALSE, (GLfloat*)&mUserData.mvpMatrix.m[0][0]);
    glUniformMatrix4fv(mUserData.uMMatrixHandle, 1, GL_FALSE, (GLfloat*)&mUserData.modelMatrix.m[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mUserData.textureId);
    glUniform1i(mUserData.uTextureHandle, 0);

    glDrawArrays ( GL_TRIANGLES, 0, numVerts );
}

void GLShaderCar::shutdown()
{
    // Delete texture object

    GLShader::shutdown();
}

GLboolean GLShaderCar::loadTexture(GLuint textureId, unsigned char *buffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    checkGlError("loadTexture");

    return TRUE;
}

