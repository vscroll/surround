//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// Hello_Triangle.c
//
//    This is a simple example that draws a single triangle with
//    a minimal vertex/fragment shader.  The purpose of this 
//    example is to demonstrate the basic concepts of 
//    OpenGL ES 2.0 rendering.
#include <stdlib.h>
#include "ogldev_mesh.h"
#include "esUtil.h"

#define STRINGIFY(A)  #A
#include "model.vert"
#include "model.frag"

typedef struct
{
    // Handle to a program object
    GLuint programObject;

    GLint positionHandle;
    GLint normalHandle;
    GLint textureCoordHandle;
		
    GLint uMVPMatrixHandle;
    GLint uMMatrixHandle;
    GLint uLightLocationHandle;

    GLint uTextureHandle;

    GLuint textureId;

    // Rotation angle
    GLfloat   angle;

    // MVP matrix
    ESMatrix mvpMatrix;
    ESMatrix modelMatrix;

    // Vertex data
    GLfloat *vertices;
    GLuint *indices;
    int numIndices;
} UserData;

const char* getVertShader()
{
    return gVShaderStr;
}

const char* getFragShader()
{
    return gFShaderStr;
}

void checkGlError(const char* op)   
{  
    GLint error;  
    for (error = glGetError(); error; error = glGetError())   
    {  
        esLogMessage("error::after %s() glError 0x%x\n", op, error);
    }  
}

int initWindow( ESContext *esContext )
{
    if (!esCreateWindow(esContext, "MultiTexture", 1024, 600, ES_WINDOW_RGB))
    {
        esLogMessage("esCreateWindow failed\n");
        return -1;
    }

    return 0;
}

///
// Initialize the shader and program object
//
int initProgram( ESContext *esContext )
{
    UserData *userData = (UserData*)(esContext->userData);

    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram(getVertShader(), getFragShader());

    // Use the program object
    glUseProgram (userData->programObject);

    checkGlError("initProgram");

    return 0;
}

void initVertexVar(ESContext *esContext)
{
    UserData *userData = (UserData*)(esContext->userData);
		
    userData->uMVPMatrixHandle = glGetUniformLocation(userData->programObject, "uMVPMatrix");

    userData->uMMatrixHandle = glGetUniformLocation(userData->programObject, "uMMatrix");
    userData->uLightLocationHandle = glGetUniformLocation(userData->programObject, "uLightLocation");

    userData->angle = 45.0f;

    checkGlError("initVertexVar");
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(ESContext *esContext)
{
    UserData *userData = (UserData *)esContext->userData;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glClear ( GL_COLOR_BUFFER_BIT );

    //glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set the viewport
    glViewport ( 0, 0, esContext->width, esContext->height );

    glEnable(GL_DEPTH_TEST);
    // If enabled, cull polygons based on their winding in window coordinates
    glEnable(GL_CULL_FACE);
    // Culls back face
    glCullFace(GL_BACK);

	
    glUniform3f(userData->uLightLocationHandle, 0, 0, 10);
    glUniformMatrix4fv(userData->uMVPMatrixHandle, 1, GL_FALSE, (GLfloat*)&userData->mvpMatrix.m[0][0]);
    glUniformMatrix4fv(userData->uMMatrixHandle, 1, GL_FALSE, (GLfloat*)&userData->modelMatrix.m[0][0]);
}

void Update ( ESContext *esContext, float deltaTime )
{
   UserData *userData = (UserData*) esContext->userData;

   ESMatrix perspective;
   float    aspect;
   
   // Compute a rotation angle based on time to rotate the cube
   userData->angle += ( deltaTime * 40.0f );
   if( userData->angle >= 360.0f )
      userData->angle -= 360.0f;

   // Compute the window aspect ratio
   aspect = (GLfloat) esContext->width / (GLfloat) esContext->height;
   
   // Generate a perspective matrix with a 60 degree FOV
   esMatrixLoadIdentity( &perspective );
   esPerspective( &perspective, 30.0f, aspect, 1.0f, 20.0f );

   // Generate a model view matrix to rotate/translate the cube
   esMatrixLoadIdentity( &userData->modelMatrix );

   // Translate away from the viewervideoSamplerLoc
   esTranslate( &userData->modelMatrix, 0.0, 0.0, -6.0 );

   // Rotate the cube
   esRotate( &userData->modelMatrix, userData->angle, 1.0, 0.0, 1.0 );
    //esRotate( &userData->modelMatrix, -90, 0.0, 1.0, 0.0 );
    //esRotate( &userData->modelMatrix, -90, 0.0, 0.0, 1.0 );
   
   // Compute the final MVP by multiplying the 
   // modevleiw and perspective matrices together
   esMatrixMultiply( &userData->mvpMatrix, &userData->modelMatrix, &perspective );
}

int main ( int argc, char *argv[] )
{
    ESContext esContext;
    esInitContext(&esContext);
    esContext.userData = new UserData();
    if ( initWindow ( &esContext ) < 0)
    {
        return 0;
    }

    initProgram(&esContext);
    initVertexVar(&esContext);

    UserData *userData = (UserData*) esContext.userData;
    Mesh* mesh = new Mesh();
    mesh->LoadMesh("./Content/box.obj");
    while (true)
    {
        Update(&esContext, 0.005);
        Draw(&esContext);
        mesh->Render();
        eglSwapBuffers(esContext.eglDisplay, esContext.eglSurface);
    }
}
