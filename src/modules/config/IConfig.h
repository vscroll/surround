#ifndef ICONFIG_20170605_H
#define ICONFIG_20170605_H

class IConfig
{
public:
    virtual ~IConfig() {}

    virtual int loadFile(char* path) = 0;
    virtual void unloadFile() = 0;

    //capture
    virtual int getChannelNo(int* front, int* rear, int* left, int* right) = 0;
    virtual int getCaptureFPS() = 0;

    //stitch
    virtual bool enableOpenCL() = 0;
    virtual int getStitchFPS() = 0;

    //render
    virtual int getRenderFPS() = 0;
    virtual int getSideRect(int* left, int* top, int* width, int* height) = 0;
    virtual int getMarkRect(int* left, int* top, int* width, int* height) = 0;
    virtual int getPanoRect(int* left, int* top, int* width, int* height) = 0;
};

#endif // ICONFIG_20170605_H

