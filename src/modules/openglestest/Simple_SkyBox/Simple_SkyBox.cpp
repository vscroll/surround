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
#include "ogldev_pipeline.h"
#include "ogldev_camera.h"
#include "mesh.h"
#include "skybox.h"

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 600

float m_scale = 0.0f;
Mesh* m_pTankMesh = NULL;    
SkyBox* m_pSkyBox = NULL;
PersProjInfo m_persProjInfo;
Camera* m_pGameCamera = NULL;

int initWindow( ESContext *esContext )
{
    if (!esCreateWindow(esContext, "MultiTexture", WINDOW_WIDTH, WINDOW_HEIGHT, ES_WINDOW_RGB))
    {
        esLogMessage("esCreateWindow failed\n");
        return -1;
    }

    return 0;
}

int initContext()
{
    m_scale = 0.0f;

    m_persProjInfo.FOV = 60.0f;
    m_persProjInfo.Width = WINDOW_WIDTH;
    m_persProjInfo.Height = WINDOW_HEIGHT;
    m_persProjInfo.zNear = 1.0f;
    m_persProjInfo.zFar = 100.0f;

    Vector3f Pos(0.0f, 1.0f, -20.0f);
    Vector3f Target(0.0f, 0.0f, 1.0f);
    Vector3f Up(0.0, 1.0f, 0.0f);
    m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);

#if 0
    m_pTankMesh = new Mesh();
    if (!m_pTankMesh->LoadMesh("./Content/phoenix_ugv.md2"))
    {
        return -1;
    }
#endif
        
    m_pSkyBox = new SkyBox(m_pGameCamera, m_persProjInfo);
    if (!m_pSkyBox->Init(".",
			"Content/sp3right.jpg",
			"Content/sp3left.jpg",
			"Content/sp3top.jpg",
			"Content/sp3bot.jpg",
			"Content/sp3front.jpg",
			"Content/sp3back.jpg"))
    {
            return -1;
    }

    return 0;
}

void Draw ( ESContext *esContext )
{
    m_pGameCamera->OnRender();
    m_scale += 0.05f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#if 0
    Pipeline p;        
    p.Scale(0.1f, 0.1f, 0.1f);
    p.Rotate(0.0f, m_scale, 0.0f);
    p.WorldPos(0.0f, -5.0f, 3.0f);
    p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
    p.SetPerspectiveProj(m_persProjInfo);

    //m_pTankMesh->Render();
#endif

    m_pSkyBox->Render();
}

int main ( int argc, char *argv[] )
{
    ESContext esContext;
    esInitContext(&esContext);
    if ( initWindow ( &esContext ) < 0)
    {
        return 0;
    }

    if ( initContext () < 0)
    {
        return 0;
    }

    esRegisterDrawFunc ( &esContext, Draw );
    esMainLoop ( &esContext );
}
