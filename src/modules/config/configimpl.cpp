#include "configimpl.h"
#include <stdio.h>
#include "inifile.h"

#define SECTION_CAPTURE         "CAPTURE"
#define CAPTURE_KEY_FRONTCHN    "FrontCHN"
#define CAPTURE_KEY_REARCHN     "RearCHN"
#define CAPTURE_KEY_LEFTCHN     "LeftCHN"
#define CAPTURE_KEY_RIGHTCHN    "RightCHN"
#define CAPTURE_KEY_FPS         "FPS"

#define SECTION_SINK	        "SINK"
#define SINK_KEY_SINKWIDTH   	"SinkWidth"
#define SINK_KEY_SINKHEIGHT  	"SinkHeight"
#define SINK_KEY_SINKPIXFMT  	"SinkPixfmt"

#define SECTION_CROP	        "CROP"
#define CROP_KEY_PREFIX       	"CHN"
#define CROP_KEY_SUFFIX_X      	"X"
#define CROP_KEY_SUFFIX_Y      	"Y"
#define CROP_KEY_SUFFIX_WIDTH   "Width"
#define CROP_KEY_SUFFIX_HEIGHT	"Height"
#define CROP_KEY_CHN0X       	(CROP_KEY_PREFIX##"0"##CROP_KEY_SUFFIX_X)
#define CROP_KEY_CHN0Y       	(CROP_KEY_PREFIX##"0"##CROP_KEY_SUFFIX_Y)
#define CROP_KEY_CHN0WIDTH   	(CROP_KEY_PREFIX##"0"##CROP_KEY_SUFFIX_WIDTH)
#define CROP_KEY_CHN0HEIGHT  	(CROP_KEY_PREFIX##"0"##CROP_KEY_SUFFIX_HEIGHT)
#define CROP_KEY_CHN1X       	(CROP_KEY_PREFIX##"1"##CROP_KEY_SUFFIX_X)
#define CROP_KEY_CHN1Y       	(CROP_KEY_PREFIX##"1"##CROP_KEY_SUFFIX_Y)
#define CROP_KEY_CHN1WIDTH   	(CROP_KEY_PREFIX##"1"##CROP_KEY_SUFFIX_WIDTH)
#define CROP_KEY_CHN1HEIGHT  	(CROP_KEY_PREFIX##"1"##CROP_KEY_SUFFIX_HEIGHT)
#define CROP_KEY_CHN2X       	(CROP_KEY_PREFIX##"2"##CROP_KEY_SUFFIX_X)
#define CROP_KEY_CHN2Y       	(CROP_KEY_PREFIX##"2"##CROP_KEY_SUFFIX_Y)
#define CROP_KEY_CHN2WIDTH   	(CROP_KEY_PREFIX##"2"##CROP_KEY_SUFFIX_WIDTH)
#define CROP_KEY_CHN2HEIGHT  	(CROP_KEY_PREFIX##"2"##CROP_KEY_SUFFIX_HEIGHT)
#define CROP_KEY_CHN3X       	(CROP_KEY_PREFIX##"3"##CROP_KEY_SUFFIX_X)
#define CROP_KEY_CHN3Y       	(CROP_KEY_PREFIX##"3"##CROP_KEY_SUFFIX_Y)
#define CROP_KEY_CHN3WIDTH   	(CROP_KEY_PREFIX##"3"##CROP_KEY_SUFFIX_WIDTH)
#define CROP_KEY_CHN3HEIGHT  	(CROP_KEY_PREFIX##"3"##CROP_KEY_SUFFIX_HEIGHT)

#define CROP_KEY_FOCUSX       	"FocusX"
#define CROP_KEY_FOCUSY       	"FocusY"
#define CROP_KEY_FOCUSWIDTH   	"FocusWidth"
#define CROP_KEY_FOCUSHEIGHT  	"FocusHeight"


#define SECTION_SOURCE	        	"SOURCE"
#define SOURCE_KEY_PREFIX       	"CHN"
#define SOURCE_KEY_SUFFIX_WIDTH   	"Width"
#define SOURCE_KEY_SUFFIX_HEIGHT	"Height"
#define SOURCE_KEY_SOURCE_PIXFMT	"SourcePixfmt"
#define SOURCE_KEY_CHN0WIDTH   		(SOURCE_KEY_PREFIX##"0"##SOURCE_KEY_SUFFIX_WIDTH)
#define SOURCE_KEY_CHN0HEIGHT  		(SOURCE_KEY_PREFIX##"0"##SOURCE_KEY_SUFFIX_HEIGHT)
#define SOURCE_KEY_CHN1WIDTH   		(SOURCE_KEY_PREFIX##"1"##SOURCE_KEY_SUFFIX_WIDTH)
#define SOURCE_KEY_CHN1HEIGHT  		(SOURCE_KEY_PREFIX##"1"##SOURCE_KEY_SUFFIX_HEIGHT)
#define SOURCE_KEY_CHN2WIDTH   		(SOURCE_KEY_PREFIX##"2"##SOURCE_KEY_SUFFIX_WIDTH)
#define SOURCE_KEY_CHN2HEIGHT  		(SOURCE_KEY_PREFIX##"2"##SOURCE_KEY_SUFFIX_HEIGHT)
#define SOURCE_KEY_CHN3WIDTH   		(SOURCE_KEY_PREFIX##"3"##SOURCE_KEY_SUFFIX_WIDTH)
#define SOURCE_KEY_CHN3HEIGHT  		(SOURCE_KEY_PREFIX##"3"##SOURCE_KEY_SUFFIX_HEIGHT)

#define SOURCE_KEY_FOCUSWIDTH   	"FocusWidth"
#define SOURCE_KEY_FOCUSHEIGHT  	"FocusHeight"

#define SECTION_STITCH          "STITCH"
#define STITCH_KEY_ACCELPOLICY  "AccelPolicy"
#define STITCH_KEY_FPS "FPS"

#define SECTION_RENDER          	"RENDER"
#define RENDER_KEY_SIDEFPS      	"SideFPS"
#define RENDER_KEY_SIDECROPLEFT     "SideCropLeft"
#define RENDER_KEY_SIDECROPTOP      "SideCropTop"
#define RENDER_KEY_SIDECROPWIDTH    "SideCropWidth"
#define RENDER_KEY_SIDECROPHEIGHT   "SideCropHeight"
#define RENDER_KEY_SIDELEFT     	"SideLeft"
#define RENDER_KEY_SIDETOP      	"SideTop"
#define RENDER_KEY_SIDEWIDTH    	"SideWidth"
#define RENDER_KEY_SIDEHEIGHT   	"SideHeight"
#define RENDER_KEY_MARKFPS      	"MarkFPS"
#define RENDER_KEY_MARKLEFT     	"MarkLeft"
#define RENDER_KEY_MARKTOP      	"MarkTop"
#define RENDER_KEY_MARKWIDTH    	"MarkWidth"
#define RENDER_KEY_MARKHEIGHT   	"MarkHeight"
#define RENDER_KEY_PANOFPS      	"PanoFPS"
#define RENDER_KEY_PANOLEFT     	"PanoLeft"
#define RENDER_KEY_PANOTOP      	"PanoTop"
#define RENDER_KEY_PANOWIDTH    	"PanoWidth"
#define RENDER_KEY_PANOHEIGHT   	"PanoHeight"

ConfigImpl::ConfigImpl()
{
    mIniFile = new CIniFile();
}

ConfigImpl::~ConfigImpl()
{
	if (NULL != mIniFile)
	{
		delete mIniFile;
		mIniFile = NULL;
	}
}

int ConfigImpl::loadFromFile(char* path)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    if (mIniFile->open(path) < 0)
	{
		return -1;
	}

	mIniFile->close();
	return 0;
}

int ConfigImpl::saveAsFile(char* path)
{
    if (mIniFile == NULL)
    {
        return -1;
    }
	
	return mIniFile->saveAsFile(path);
}

//capture
int ConfigImpl::getChannelNo(int* front, int* rear, int* left, int* right)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    *front = mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_FRONTCHN);
    *rear = mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_REARCHN);
    *left = mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_LEFTCHN);
    *right = mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_RIGHTCHN);

    return 0;
}

int ConfigImpl::getCaptureFPS()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_FPS);
}

//sink
int ConfigImpl::getSinkWidth()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_SINK, SINK_KEY_SINKWIDTH);
}

int ConfigImpl::getSinkHeight()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_SINK, SINK_KEY_SINKHEIGHT);
}

int ConfigImpl::getSinkPixfmt()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_SINK, SINK_KEY_SINKPIXFMT);
}

//crop
int ConfigImpl::getSinkCropX(int channelIndex)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_X);
    return mIniFile->getInt(SECTION_CROP, key);
}

int ConfigImpl::getSinkCropY(int channelIndex)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_Y);
    return mIniFile->getInt(SECTION_CROP, key);
}

int ConfigImpl::getSinkCropWidth(int channelIndex)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_WIDTH);
    return mIniFile->getInt(SECTION_CROP, key);
}

int ConfigImpl::getSinkCropHeight(int channelIndex)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_HEIGHT);
    return mIniFile->getInt(SECTION_CROP, key);
}

int ConfigImpl::getFocusSinkCropX()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CROP, CROP_KEY_FOCUSX);
}

int ConfigImpl::getFocusSinkCropY()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CROP, CROP_KEY_FOCUSY);
}

int ConfigImpl::getFocusSinkCropWidth()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CROP, CROP_KEY_FOCUSWIDTH);
}

int ConfigImpl::getFocusSinkCropHeight()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CROP, CROP_KEY_FOCUSHEIGHT);
}

int ConfigImpl::setSinkCropX(int channelIndex, int x)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_X);
    return mIniFile->setInt(SECTION_CROP, key, x);
}


int ConfigImpl::setSinkCropY(int channelIndex, int y)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_Y);
    return mIniFile->setInt(SECTION_CROP, key, y);
}

int ConfigImpl::setSinkCropWidth(int channelIndex, int width)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_WIDTH);
    return mIniFile->setInt(SECTION_CROP, key, width);
}

int ConfigImpl::setSinkCropHeight(int channelIndex, int height)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", CROP_KEY_PREFIX, channelIndex, CROP_KEY_SUFFIX_HEIGHT);
    return mIniFile->setInt(SECTION_CROP, key, height);
}

int ConfigImpl::setFocusSinkCropX(int x)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->setInt(SECTION_CROP, CROP_KEY_FOCUSX, x);
}


int ConfigImpl::setFocusSinkCropY(int y)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->setInt(SECTION_CROP, CROP_KEY_FOCUSY, y);
}

int ConfigImpl::setFocusSinkCropWidth(int width)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->setInt(SECTION_CROP, CROP_KEY_FOCUSWIDTH, width);
}

int ConfigImpl::setFocusSinkCropHeight(int height)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->setInt(SECTION_CROP, CROP_KEY_FOCUSHEIGHT, height);
}

//source
int ConfigImpl::getSourceWidth(int channelIndex)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", SOURCE_KEY_PREFIX, channelIndex, SOURCE_KEY_SUFFIX_WIDTH);
    return mIniFile->getInt(SECTION_SOURCE, key);
}

int ConfigImpl::getSourceHeight(int channelIndex)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

	char key[256] = {0};
	sprintf(key, "%s%d%s", SOURCE_KEY_PREFIX, channelIndex, SOURCE_KEY_SUFFIX_HEIGHT);
    return mIniFile->getInt(SECTION_SOURCE, key);
}

int ConfigImpl::getSourcePixfmt()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_SOURCE, SOURCE_KEY_SOURCE_PIXFMT);
}

int ConfigImpl::getFocusSourceWidth()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_SOURCE, SOURCE_KEY_FOCUSWIDTH);
}

int ConfigImpl::getFocusSourceHeight()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_SOURCE, SOURCE_KEY_FOCUSHEIGHT);
}

//stitch
int ConfigImpl::getAccelPolicy()
{
    if (mIniFile == NULL)
    {
        return false;
    }

    return mIniFile->getInt(SECTION_STITCH, STITCH_KEY_ACCELPOLICY);    
}

int ConfigImpl::getStitchFPS()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_STITCH, STITCH_KEY_FPS);
}

//render
int ConfigImpl::getSideFPS()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDEFPS);
}

int ConfigImpl::getMarkFPS()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_RENDER, RENDER_KEY_MARKFPS);
}

int ConfigImpl::getPanoFPS()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_RENDER, RENDER_KEY_PANOFPS);
}

int ConfigImpl::getSideCrop(int* left, int* top, int* width, int* height)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    *left = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDECROPLEFT);
    *top = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDECROPTOP);
    *width = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDECROPWIDTH);
    *height = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDECROPHEIGHT);

    return 0;
}

int ConfigImpl::getSideRect(int* left, int* top, int* width, int* height)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    *left = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDELEFT);
    *top = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDETOP);
    *width = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDEWIDTH);
    *height = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_SIDEHEIGHT);

    return 0;
}

int ConfigImpl::getMarkRect(int* left, int* top, int* width, int* height)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    *left = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_MARKLEFT);
    *top = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_MARKTOP);
    *width = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_MARKWIDTH);
    *height = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_MARKHEIGHT);

    return 0;
}

int ConfigImpl::getPanoRect(int* left, int* top, int* width, int* height)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    *left = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_PANOLEFT);
    *top = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_PANOTOP);
    *width = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_PANOWIDTH);
    *height = mIniFile->getInt(SECTION_RENDER, RENDER_KEY_PANOHEIGHT);

    return 0;
}
