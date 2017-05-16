#ifndef RENDERBASE_H
#define RENDERBASE_H

class RenderDevice;
class RenderBase
{
public:
    RenderBase();
    virtual ~RenderBase();
    virtual int openDevice(unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height);

    virtual void closeDevice();

    virtual void drawImage(unsigned char* buf, unsigned int size);

private:
    RenderDevice* mRenderDevice;
};

#endif // RENDERBASE_H
