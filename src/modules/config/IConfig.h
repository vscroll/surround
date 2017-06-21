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
    virtual int getSinkWidth() = 0;
    virtual int getSinkHeight() = 0;
    virtual int getSinkCropX() = 0;
    virtual int getSinkCropY() = 0;
    virtual int getSinkCropWidth() = 0;
    virtual int getSinkCropHeight() = 0;
    virtual int getSrcWidth() = 0;
    virtual int getSrcHeight() = 0;

    //stitch
    virtual bool enableOpenCL() = 0;
    virtual int getStitchFPS() = 0;

    //render
    virtual int getSideFPS() = 0;
    virtual int getMarkFPS() = 0;
    virtual int getPanoFPS() = 0;
    virtual int getSideRect(int* left, int* top, int* width, int* height) = 0;
    virtual int getMarkRect(int* left, int* top, int* width, int* height) = 0;
    virtual int getPanoRect(int* left, int* top, int* width, int* height) = 0;
};

#endif // ICONFIG_20170605_H

