#ifndef IRENDER_H
#define IRENDER_H

#include <stdio.h>

class ICapture;
class IPanoImage;
class IRender
{
public:
    virtual ~IRender() {}

    virtual void setCaptureModule(ICapture* capture = NULL) = 0;
    virtual void setSideImageCrop(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height) = 0;

    virtual void setSideImageRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height) = 0;

    virtual void setMarkRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height) = 0;

    virtual void setPanoImageModule(IPanoImage* panoImage = NULL) = 0;
    virtual void setPanoImageRect(
        unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height) = 0;

    virtual int startRenderSide(unsigned int fps) = 0;
    virtual int startRenderMark(unsigned int fps) = 0;
    virtual int startRenderPano(unsigned int fps) = 0;

    virtual int start(unsigned int fps) = 0;
    virtual void stop() = 0;
};

#endif // IRENDER_H
