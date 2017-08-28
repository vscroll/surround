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
#include "ogldev_mesh.h"
#include "ogldev_pipeline.h"
#include "skybox.h"

Mesh* m_pTankMesh;    
SkyBox* m_pSkyBox;
PersProjInfo m_persProjInfo;
float m_scale;

int initWindow( ESContext *esContext )
{
    if (!esCreateWindow(esContext, "MultiTexture", 1024, 600, ES_WINDOW_RGB))
    {
        esLogMessage("esCreateWindow failed\n");
        return -1;
    }

    return 0;
}

void Draw ( ESContext *esContext )
{
    m_scale += 0.05f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Pipeline p;        
    p.Scale(0.1f, 0.1f, 0.1f);
    p.Rotate(0.0f, m_scale, 0.0f);
    p.WorldPos(0.0f, -5.0f, 3.0f);
    //p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());
    p.SetPerspectiveProj(m_persProjInfo);

    m_pTankMesh->Render();
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

    m_pTankMesh = NULL;
    m_pSkyBox = NULL;

    m_persProjInfo.FOV = 60.0f;
    m_persProjInfo.Height = 1024;
    m_persProjInfo.Width = 600;
    m_persProjInfo.zNear = 1.0f;
    m_persProjInfo.zFar = 100.0f;

    m_scale = 0.0f;

    m_pTankMesh = new Mesh();
    if (!m_pTankMesh->LoadMesh("./Content/phoenix_ugv.md2"))
    {
        return false;
    }
        
    m_pSkyBox = new SkyBox(m_persProjInfo);
    if (!m_pSkyBox->Init(".",
			"./Content/sp3right.jpg",
			"./Content/sp3left.jpg",
			"./Content/sp3top.jpg",
			"./Content/sp3bot.jpg",
			"./Content/sp3front.jpg",
			"./Content/sp3back.jpg"))
    {
            return false;
    }

    esRegisterDrawFunc ( &esContext, Draw );
    esMainLoop ( &esContext );
}
