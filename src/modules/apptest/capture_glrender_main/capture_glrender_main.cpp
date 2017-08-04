#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "common.h"
#include <linux/input.h>

#include "controllergl.h"

int main (int argc, char **argv)
{
    ControllerGL controller;
    if (controller.initConfigModule() < 0)
    {
        return -1;
    }

    if (controller.startCaptureModule(false) < 0)
    {
        goto failed;
    }

    if (controller.startInputEventModule() < 0)
    {
        goto failed;
    }
    
    if (controller.startGLRenderModule() < 0)
    {
        goto failed;
    }

    while (true)
    {
        sleep(100);
    }

failed:
    controller.stopInputEventModule();
    controller.stopGLRenderModule();
    controller.stopCaptureModule();
    controller.uninitConfigModule();

    return 0;
}
