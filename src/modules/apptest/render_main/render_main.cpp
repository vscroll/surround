// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "common.h"
#include "IRender.h"
#include "renderimpl.h"

int main (int argc, char **argv)
{
    RenderImpl* render = new RenderImpl();
    render->setCaptureModule(NULL);
    render->setSideImageRect(424, 0, 600, 600);
    render->setChannelMarkRect(524, 0, 100, 100);

    render->setPanoImageModule(NULL);          
    render->setPanoImageRect(0, 0, 424, 600);

    render->start(VIDEO_FPS_15);

    while (true)
    {
         sleep(10000);
    }

    return 0;
}
