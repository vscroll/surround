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
    render->init(NULL, NULL,
            424, 0, 600, 600,
            0, 0, 424, 600);
    render->start(VIDEO_FPS_15);

    while (true)
    {
         sleep(10000);
    }

    return 0;
}
