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
#include "esUtil.h"
#include "banana.h"
#include "car.h"
#include "house.h"

#define TEST 0

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

    // Vertex daata
    GLfloat *vertices;
    GLuint *indices;
    int numIndices;
} UserData;

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = (char*)malloc (sizeof(char) * infoLen );

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         esLogMessage ( "Error compiling shader:\n%s\n", infoLog );            
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}

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

///
// Initialize the shader and program object
//
int initProgram( ESContext *esContext )
{
    esInitContext(esContext);
    esContext->userData = malloc(sizeof(UserData));
    UserData *userData = (UserData*)(esContext->userData);

    if (!esCreateWindow(esContext, "MultiTexture", 1024, 600, ES_WINDOW_RGB))
    {
        esLogMessage("esCreateWindow failed\n");
        return -1;
    }

    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram(getVertShader(), getFragShader());

    // Use the program object
    glUseProgram (userData->programObject);

    return 0;
}

void initVertex(ESContext *esContext)
{
    UserData *userData = (UserData*)(esContext->userData);
#if (!TEST)
	userData->positionHandle = glGetAttribLocation(userData->programObject, "aPosition");
    userData->normalHandle = glGetAttribLocation(userData->programObject, "aNormal");
    userData->textureCoordHandle = glGetAttribLocation(userData->programObject, "aTextureCoord");

		
    userData->uMVPMatrixHandle = glGetUniformLocation(userData->programObject, "uMVPMatrix");

    userData->uMMatrixHandle = glGetUniformLocation(userData->programObject, "uMMatrix");
    userData->uLightLocationHandle = glGetUniformLocation(userData->programObject, "uLightLocation");
#else
	userData->positionHandle = glGetAttribLocation(userData->programObject, "aPosition");
    userData->uMVPMatrixHandle = glGetUniformLocation(userData->programObject, "uMVPMatrix");

    userData->numIndices = esGenCube( 1.0, &userData->vertices,
                                     NULL, NULL, &userData->indices );
#endif

    userData->angle = 45.0f;

    checkGlError("initTexture");
}

void loadTexture(GLuint textureId, unsigned char *buffer, int width, int height)
{
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    checkGlError("loadTexture");
}

void initTexture(ESContext *esContext)
{
    UserData *userData = (UserData*)(esContext->userData);

    userData->uTextureHandle = glGetUniformLocation(userData->programObject, "uTexture");

    glGenTextures(1, &userData->textureId);

    int width;
    int height;
    char *buffer = esLoadTGA("banana.tga", &width, &height);
    loadTexture(userData->textureId, buffer, width, height);
    free (buffer);

    checkGlError("initTexture");
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

#if (!TEST)

#if 0
    float* verts = carVerts;
    float* normals = carNormals;
    float* texCoords = carTexCoords;
    unsigned int numVerts = carNumVerts;
#endif

#if 1
    float* verts = bananaVerts;
    float* normals = bananaNormals;
    float* texCoords = bananaTexCoords;
    unsigned int numVerts = bananaNumVerts;
#endif

#if 0
    float* verts = houseVerts;
    float* normals = houseNormals;
    float* texCoords = houseTexCoords;
    unsigned int numVerts = houseNumVerts;
#endif

    glVertexAttribPointer(userData->positionHandle, 3,  GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), verts);
    glVertexAttribPointer(userData->normalHandle, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), normals);
    glVertexAttribPointer(userData->textureCoordHandle, 2,  GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), texCoords);

    glEnableVertexAttribArray(userData->positionHandle);
    glEnableVertexAttribArray(userData->normalHandle);
    glEnableVertexAttribArray(userData->textureCoordHandle);
	
    glUniform3f(userData->uLightLocationHandle, 0, 0, 10);
    glUniformMatrix4fv(userData->uMVPMatrixHandle, 1, GL_FALSE, (GLfloat*)&userData->mvpMatrix.m[0][0]);
    glUniformMatrix4fv(userData->uMMatrixHandle, 1, GL_FALSE, (GLfloat*)&userData->modelMatrix.m[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId);
    glUniform1i(userData->uTextureHandle, 0);

    glDrawArrays ( GL_TRIANGLES, 0, numVerts );
#else
    // Load the vertex position
    glVertexAttribPointer ( userData->positionHandle, 3, GL_FLOAT, 
                           GL_FALSE, 3 * sizeof(GLfloat), userData->vertices );
   
    glEnableVertexAttribArray ( userData->positionHandle );
   
   
    // Load the MVP matrix
    glUniformMatrix4fv( userData->uMVPMatrixHandle, 1, GL_FALSE, (GLfloat*) &userData->mvpMatrix.m[0][0] );
   
    // Draw the cube
    glDrawElements ( GL_TRIANGLES, userData->numIndices, GL_UNSIGNED_INT, userData->indices );
#endif
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

   // Translate away from the viewer
   esTranslate( &userData->modelMatrix, 0.0, 0.0, -2.0 );

   // Rotate the cube
   esRotate( &userData->modelMatrix, userData->angle, 1.0, 0.0, 1.0 );
   
   // Compute the final MVP by multiplying the 
   // modevleiw and perspective matrices together
   esMatrixMultiply( &userData->mvpMatrix, &userData->modelMatrix, &perspective );
}

int main ( int argc, char *argv[] )
{
    ESContext esContext;
    if ( initProgram ( &esContext ) < 0)
    {
        return 0;
    }

    initVertex(&esContext);
    initTexture(&esContext);

    esRegisterUpdateFunc ( &esContext, Update );
    esRegisterDrawFunc ( &esContext, Draw );

    esMainLoop ( &esContext );
}
