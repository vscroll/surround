#ifndef IRENDER_H
#define IRENDER_H

class IStitch;
class IRender
{
public:
    virtual ~IRender() {}
    virtual int start(IStitch *stitch,
                unsigned int fps,
		unsigned int pano2DLeft,
		unsigned int pano2DTop,
		unsigned int pano2DWidth,
		unsigned int pano2DHeight,
		unsigned int sideLeft,
		unsigned int sideTop,
		unsigned int sideWidth,
		unsigned int sideHeight) = 0;
    virtual void stop() = 0;
};

#endif // IRENDER_H
