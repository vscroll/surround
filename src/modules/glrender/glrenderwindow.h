#ifndef GLRENDERWINDOW_H
#define GLRENDERWINDOW_H

#include "esUtil.h"

class GLRenderWindow
{
public:
    GLRenderWindow();
    virtual ~GLRenderWindow();

public:
    int create(int devIndex);

public:
    ESContext mESContext;
};

#endif // GLRENDERWINDOW_H
