#ifndef IGLRENDERWORKER_H
#define IGLRENDERWORKER_H

class ICapture;
class IGLRenderWorker
{
public:
    virtual ~IGLRenderWorker(){};

    virtual void setDisplayMode(unsigned int displayMode) = 0;
    virtual void setPanoramaViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height) = 0;
    virtual void setXViewRect(unsigned int left,
        unsigned int top,
        unsigned int width,
        unsigned int height) = 0;
    virtual int init(ICapture* capture) = 0;

    virtual void draw() = 0;
};

#endif // IGLRENDERWORKER_H
