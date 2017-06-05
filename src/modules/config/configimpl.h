#ifndef CONFIGIMPL_20170605_H
#define CONFIGIMPL_20170605_H

#include "IConfig.h"

class CIniFile;
class ConfigImpl : public IConfig
{
public:
    ConfigImpl();
    virtual ~ConfigImpl();

    virtual int loadFile(char* path);
    virtual void unloadFile();

    //capture
    virtual int getChannelNo(int* front, int* rear, int* left, int* right);
    virtual int getCaptureFPS();

    //stitch
    virtual bool enableOpenCL();
    virtual int getStitchFPS();

    //render
    virtual int getRenderFPS();
    virtual int getSideRect(int* left, int* top, int* width, int* height);
    virtual int getMarkRect(int* left, int* top, int* width, int* height);
    virtual int getPanoRect(int* left, int* top, int* width, int* height);

private:
    CIniFile* mIniFile;
};

#endif // CONFIGIMPL_20170605_H
