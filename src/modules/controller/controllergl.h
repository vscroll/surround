#ifndef CONTROLLERGL_H
#define CONTROLLERGL_H

#include "controller.h"

class IGLRender;
class ControllerGL : public Controller
{
public:
    ControllerGL();
    virtual ~ControllerGL();

    int startGLRenderModule();   
    void stopGLRenderModule();

private:
    IGLRender* mGLRender;
};

#endif // CONTROLLERGL_H
