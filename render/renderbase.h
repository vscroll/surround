#ifndef RENDERBASE_H
#define RENDERBASE_H

class IStitch;
class RenderDevice;
class RenderBase
{
public:
    RenderBase();
    virtual ~RenderBase();
    virtual int init(IStitch *stitch,
		unsigned int left,
		unsigned int top,
		unsigned int width,
		unsigned int height);

    virtual void uninit();

    virtual void drawImage(unsigned char* buf, unsigned int size);

protected:
    IStitch* mStitch;

private:
    RenderDevice* mRenderDevice;
};

#endif // RENDERBASE_H
