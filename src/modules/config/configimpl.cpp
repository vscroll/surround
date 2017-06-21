#include "configimpl.h"
#include "inifile.h"

#define SECTION_CAPTURE         "CAPTURE"
#define CAPTURE_KEY_FRONTCHN    "FrontCHN"
#define CAPTURE_KEY_REARCHN     "RearCHN"
#define CAPTURE_KEY_LEFTCHN     "LeftCHN"
#define CAPTURE_KEY_RIGHTCHN    "RightCHN"
#define CAPTURE_KEY_FPS         "FPS"
#define CAPTURE_KEY_SINKWIDTH   "SinkWidth"
#define CAPTURE_KEY_SINKHEIGHT  "SinkHeight"
#define CAPTURE_KEY_CROPX       "CropX"
#define CAPTURE_KEY_CROPY       "CropY"
#define CAPTURE_KEY_CROPWIDTH   "CropWidth"
#define CAPTURE_KEY_CROPHEIGHT  "CropHeight"
#define CAPTURE_KEY_SRCWIDTH    "SrcWidth"
#define CAPTURE_KEY_SRCHEIGHT   "SrcHeight"

#define SECTION_STITCH          "STITCH"
#define STITCH_KEY_ENABLEOPENCL "EnableOpenCL"
#define STITCH_KEY_FPS "FPS"

#define SECTION_RENDER          "RENDER"
#define RENDER_KEY_SIDEFPS      "SideFPS"
#define RENDER_KEY_SIDELEFT     "SideLeft"
#define RENDER_KEY_SIDETOP      "SideTop"
#define RENDER_KEY_SIDEWIDTH    "SideWidth"
#define RENDER_KEY_SIDEHEIGHT   "SideHeight"
#define RENDER_KEY_MARKFPS      "MarkFPS"
#define RENDER_KEY_MARKLEFT     "MarkLeft"
#define RENDER_KEY_MARKTOP      "MarkTop"
#define RENDER_KEY_MARKWIDTH    "MarkWidth"
#define RENDER_KEY_MARKHEIGHT   "MarkHeight"
#define RENDER_KEY_PANOFPS      "PanoFPS"
#define RENDER_KEY_PANOLEFT     "PanoLeft"
#define RENDER_KEY_PANOTOP      "PanoTop"
#define RENDER_KEY_PANOWIDTH    "PanoWidth"
#define RENDER_KEY_PANOHEIGHT   "PanoHeight"

ConfigImpl::ConfigImpl()
{
    mIniFile = new CIniFile();
}

ConfigImpl::~ConfigImpl()
{
}

int ConfigImpl::loadFile(char* path)
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->open(path);
}

void ConfigImpl::unloadFile()
{
    if (mIniFile != NULL)
    {
        mIniFile->close();
    }   
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

int ConfigImpl::getSinkWidth()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_SINKWIDTH);
}

int ConfigImpl::getSinkHeight()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_SINKHEIGHT);
}

int ConfigImpl::getSinkCropX()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_CROPX);
}

int ConfigImpl::getSinkCropY()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_CROPY);
}

int ConfigImpl::getSinkCropWidth()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_CROPWIDTH);
}

int ConfigImpl::getSinkCropHeight()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_CROPHEIGHT);
}

int ConfigImpl::getSrcWidth()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_SRCWIDTH);
}

int ConfigImpl::getSrcHeight()
{
    if (mIniFile == NULL)
    {
        return -1;
    }

    return mIniFile->getInt(SECTION_CAPTURE, CAPTURE_KEY_SRCHEIGHT);
}

//stitch
bool ConfigImpl::enableOpenCL()
{
    if (mIniFile == NULL)
    {
        return false;
    }

    return (mIniFile->getInt(SECTION_STITCH, STITCH_KEY_ENABLEOPENCL) == 1);    
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
