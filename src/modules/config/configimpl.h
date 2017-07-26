#ifndef CONFIGIMPL_20170605_H
#define CONFIGIMPL_20170605_H

#include "IConfig.h"

class CIniFile;
class ConfigImpl : public IConfig
{
public:
    ConfigImpl();
    virtual ~ConfigImpl();

    virtual int loadFromFile(char* path);
    virtual int saveAsFile(char* path);

    //capture
    virtual int getChannelNo(int* front, int* rear, int* left, int* right);
    virtual int getCaptureFPS();

	//sink
    virtual int getSinkWidth();
    virtual int getSinkHeight();

	//crop
    virtual int getSinkCropX(int channelIndex);
    virtual int getSinkCropY(int channelIndex);
    virtual int getSinkCropWidth(int channelIndex);
    virtual int getSinkCropHeight(int channelIndex);

    virtual int getFocusSinkCropX();
    virtual int getFocusSinkCropY();
    virtual int getFocusSinkCropWidth();
    virtual int getFocusSinkCropHeight();

    virtual int setSinkCropX(int channelIndex, int x);
    virtual int setSinkCropY(int channelIndex, int y);
    virtual int setSinkCropWidth(int channelIndex, int width);
    virtual int setSinkCropHeight(int channelIndex, int height);

    virtual int setFocusSinkCropX(int x);
    virtual int setFocusSinkCropY(int y);
    virtual int setFocusSinkCropWidth(int width);
    virtual int setFocusSinkCropHeight(int height);

	//source
    virtual int getSourceWidth(int channelIndex);
    virtual int getSourceHeight(int channelIndex);

    virtual int getFocusSourceWidth();
    virtual int getFocusSourceHeight();

    //stitch
    virtual int getAccelPolicy();
    virtual int getStitchFPS();

    //render
    virtual int getSideFPS();
    virtual int getMarkFPS();
    virtual int getPanoFPS();
	virtual int getSideCrop(int* left, int* top, int* width, int* height);
    virtual int getSideRect(int* left, int* top, int* width, int* height);
    virtual int getMarkRect(int* left, int* top, int* width, int* height);
    virtual int getPanoRect(int* left, int* top, int* width, int* height);

private:
    CIniFile* mIniFile;
};

#endif // CONFIGIMPL_20170605_H
