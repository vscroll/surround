//
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com
//

// MultiTexture.c
//
//    This is an example that draws a quad with a basemap and
//    lightmap to demonstrate multitexturing.
//
#include <stdlib.h>
#include "esUtil.h"

extern unsigned char yuyv_352x288[];
extern unsigned char Front_uyvy_352x288[];
extern unsigned char NV16_352x288[];
extern unsigned char akiyo_I420_176x144[];

static const char* vShaderStr =  
    "attribute vec4 a_position;   \n"
    "attribute vec2 a_texCoord;   \n"
    "varying vec2 v_texCoord;     \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = a_position; \n"
    "   v_texCoord = a_texCoord;  \n"
    "}                            \n";
   
static const char* fShaderStr_I420_0 =  
    "precision mediump float;                             \n"
    "varying lowp vec2 v_texCoord;                        \n"
    "uniform sampler2D s_samplerY;                        \n"
    "uniform sampler2D s_samplerU;                        \n"
    "uniform sampler2D s_samplerV;                        \n"
    "void main(void)                                      \n"
    "{                                                    \n"
    "   mediump vec3 yuv;                                 \n"
    "   lowp vec3 rgb;                                    \n"
    "   yuv.x = texture2D(s_samplerY, v_texCoord).r;      \n"
    "   yuv.y = texture2D(s_samplerU, v_texCoord).r - 0.5;\n"
    "   yuv.z = texture2D(s_samplerV, v_texCoord).r - 0.5;\n"
    "   rgb = mat3( 1,   1,   1,                          \n"
    "               0,         -0.39465,  2.03211,        \n"
    "               1.13983,   -0.58060,  0) * yuv;       \n"
    "   gl_FragColor = vec4(rgb, 1);                      \n"
    "}                                                    \n";

static const char* fShaderStr_I420_1 =    
    "precision mediump float;                                                       \n"
    "varying lowp vec2 v_texCoord;                                                  \n"
    "uniform sampler2D s_samplerY;                                                  \n"
    "uniform sampler2D s_samplerU;                                                  \n"
    "uniform sampler2D s_samplerV;                                                  \n"
    "void main()                                                                    \n"
    "{                                                                              \n"
    "    vec4 c = vec4((texture2D(s_samplerY, v_texCoord).r - 16.0/255.0) * 1.164); \n"
    "    vec4 U = vec4(texture2D(s_samplerU, v_texCoord).r - 128.0/255.0);          \n"
    "    vec4 V = vec4(texture2D(s_samplerV, v_texCoord).r - 128.0/255.0);          \n"
    "    c += V * vec4(1.596, -0.813, 0, 0);                                        \n"
    "    c += U * vec4(0, -0.392, 2.017, 0);                                        \n"
    "    c.a = 1.0;                                                                 \n"
    "    gl_FragColor = c;                                                          \n"
    "}                                                                              \n";

typedef struct
{
    // Handle to a program object
    GLuint programObject;

    // Attribute locations
    GLint  positionLoc;
    GLint  texCoordLoc;

    // Sampler locations
    GLint ySamplerLoc;
    GLint uSamplerLoc;
    GLint vSamplerLoc;

    // Texture handle
    GLuint yTexId;
    GLuint uTexId;
    GLuint vTexId;

} UserData;


///
// Load texture from disk
//
GLboolean BindTexture ( GLuint textureId, const char *buffer, GLuint width, GLuint height )
{
    glBindTexture ( GL_TEXTURE_2D, textureId );

    glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    return TRUE;
}



///
// Initialize the shader and program object
//
int Init ( ESContext *esContext )
{
    UserData *userData = esContext->userData;

    char* fShaderStr = fShaderStr_I420_0;

    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram ( vShaderStr, fShaderStr );

    // Use the program object
    glUseProgram ( userData->programObject );

    // Get the attribute locations
    userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
    userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );
   
    // Get the sampler location
    userData->ySamplerLoc = glGetUniformLocation ( userData->programObject, "s_samplerY" );
    userData->uSamplerLoc = glGetUniformLocation ( userData->programObject, "s_samplerU" );
    userData->vSamplerLoc = glGetUniformLocation ( userData->programObject, "s_samplerV" );

    glGenTextures(1, &userData->yTexId);
    glGenTextures(1, &userData->uTexId);
    glGenTextures(1, &userData->vTexId);

    if ( userData->yTexId == 0
        || userData->uTexId == 0
        || userData->vTexId == 0)
    {
        return FALSE;
    }

    glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );

    return TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( ESContext *esContext )
{
   UserData *userData = esContext->userData;
   
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

   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
      
   // Set the viewport
   glViewport ( 0, 0, esContext->width, esContext->height );

#if 0
   GLuint width = 352;
   GLuint height = 288;

   unsigned char* buffer = yuyv_352x288;
   //unsigned char* buffer = Front_uyvy_352x288;
   //unsigned char* buffer = NV16_352x288;
#else
   GLuint width = 176;
   GLuint height = 144;
   unsigned char* buffer = akiyo_I420_176x144;
#endif

   // bind the textures
   BindTexture(userData->yTexId, buffer, width, height);  
   BindTexture(userData->uTexId, buffer + width * height, width/2, height/2);  
   BindTexture(userData->vTexId, buffer + width * height * 5 / 4, width/2, height/2);  
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Load the vertex position
   glVertexAttribPointer ( userData->positionLoc, 2, GL_FLOAT, 
                           GL_FALSE, 2 * sizeof(GLfloat), squareVertices );
   // Load the texture coordinate
   glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 2 * sizeof(GLfloat), coordVertices );

   glEnableVertexAttribArray ( userData->positionLoc );
   glEnableVertexAttribArray ( userData->texCoordLoc );

   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->yTexId );
   glUniform1i ( userData->ySamplerLoc, 0 );

   glActiveTexture ( GL_TEXTURE1 );
   glBindTexture ( GL_TEXTURE_2D, userData->uTexId );
   glUniform1i ( userData->uSamplerLoc, 1 );

   glActiveTexture ( GL_TEXTURE2 );
   glBindTexture ( GL_TEXTURE_2D, userData->vTexId );
   glUniform1i ( userData->vSamplerLoc, 2 );

   //glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   eglSwapBuffers ( esContext->eglDisplay, esContext->eglSurface );
}

///
// Cleanup
//
void ShutDown ( ESContext *esContext )
{
   UserData *userData = esContext->userData;

   // Delete texture object
   glDeleteTextures ( 1, &userData->yTexId );
   glDeleteTextures ( 1, &userData->uTexId );
   glDeleteTextures ( 1, &userData->vTexId );

   // Delete program object
   glDeleteProgram ( userData->programObject );
}


int main ( int argc, char *argv[] )
{
   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   if (!esCreateWindow ( &esContext, "MultiTexture", 1024, 600, ES_WINDOW_RGB ))
   {
       esLogMessage ( "Error esCreateWindow.\n");
   }
   
   if ( !Init ( &esContext ) )
      return 0;

   esRegisterDrawFunc ( &esContext, Draw );
   
   esMainLoop ( &esContext );

   ShutDown ( &esContext );
}
