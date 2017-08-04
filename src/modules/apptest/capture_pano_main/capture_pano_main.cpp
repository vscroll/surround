#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "common.h"
#include "controllercl.h"

int main (int argc, char **argv)
{
    ControllerCL controller;
    if (controller.initConfigModule() < 0)
    {
        goto failed;
    }

    if (controller.startCaptureModule(false) < 0)
    {
        goto failed;
    }

    
    if (controller.startPanoImageModule() < 0)
    {
        goto failed;
    }

    if (controller.startInputEventModule() < 0)
    {
        goto failed;
    }

    while (true)
    {
        sleep(100);
    }

failed:
    controller.stopInputEventModule();
    controller.stopPanoImageModule();
    controller.stopCaptureModule();
    controller.uninitConfigModule();

    return 0;
}
