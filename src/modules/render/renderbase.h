#ifndef RENDERBASE_H
#define RENDERBASE_H

class RenderDevice;
class RenderBase
{
public:
    RenderBase();
    virtual ~RenderBase();
    virtual int openDevice(unsigned int dstLeft,
		unsigned int dstTop,
		unsigned int dstWidth,
		unsigned int dstHeight);

    virtual void closeDevice();

    virtual void drawImage(unsigned char* buf,
            unsigned int srcPixfmt,
            unsigned int srcWidth,
            unsigned int srcHeight,
            unsigned int srcSize);

private:
    RenderDevice* mRenderDevice;
};

#endif // RENDERBASE_H
