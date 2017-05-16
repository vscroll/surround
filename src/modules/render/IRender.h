#ifndef IRENDER_H
#define IRENDER_H

class ICapture;
class IPanoImage;
class IRender
{
public:
    virtual ~IRender() {}
    virtual int init(
        ICapture* capture,
        IPanoImage* panoImage,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight,
		unsigned int panoLeft,
		unsigned int panoTop,
		unsigned int panoWidth,
		unsigned int panoHeight) = 0;
    virtual int start(unsigned int fps) = 0;
    virtual void stop() = 0;
};

#endif // IRENDER_H
