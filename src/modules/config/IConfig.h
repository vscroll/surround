#ifndef ICONFIG_20170605_H
#define ICONFIG_20170605_H

class IConfig
{
public:
    virtual ~IConfig() {}

    virtual int loadFromFile(char* path) = 0;
    virtual int saveAsFile(char* path) = 0;

    //capture
    virtual int getChannelNo(int* front, int* rear, int* left, int* right) = 0;
    virtual int getCaptureFPS() = 0;

	//sink
    virtual int getSinkWidth() = 0;
    virtual int getSinkHeight() = 0;

	//crop
    virtual int getSinkCropX(int channelIndex) = 0;
    virtual int getSinkCropY(int channelIndex) = 0;
    virtual int getSinkCropWidth(int channelIndex) = 0;
    virtual int getSinkCropHeight(int channelIndex) = 0;

    virtual int getFocusSinkCropX() = 0;
    virtual int getFocusSinkCropY() = 0;
    virtual int getFocusSinkCropWidth() = 0;
    virtual int getFocusSinkCropHeight() = 0;

    virtual int setSinkCropX(int channelIndex, int x) = 0;
    virtual int setSinkCropY(int channelIndex, int y) = 0;
    virtual int setSinkCropWidth(int channelIndex, int width) = 0;
    virtual int setSinkCropHeight(int channelIndex, int height) = 0;

    virtual int setFocusSinkCropX(int x) = 0;
    virtual int setFocusSinkCropY(int y) = 0;
    virtual int setFocusSinkCropWidth(int width) = 0;
    virtual int setFocusSinkCropHeight(int height) = 0;

	//source
    virtual int getSourceWidth(int channelIndex) = 0;
    virtual int getSourceHeight(int channelIndex) = 0;

    virtual int getFocusSourceWidth() = 0;
    virtual int getFocusSourceHeight() = 0;

    //stitch
    virtual int getAccelPolicy() = 0;
    virtual int getStitchFPS() = 0;

    //render
    virtual int getSideFPS() = 0;
    virtual int getMarkFPS() = 0;
    virtual int getPanoFPS() = 0;
	virtual int getSideCrop(int* left, int* top, int* width, int* height) = 0;
    virtual int getSideRect(int* left, int* top, int* width, int* height) = 0;
    virtual int getMarkRect(int* left, int* top, int* width, int* height) = 0;
    virtual int getPanoRect(int* left, int* top, int* width, int* height) = 0;
};

#endif // ICONFIG_20170605_H

