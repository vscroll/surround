#ifndef IGLRENDER_H
#define IGLRENDER_H

class ICapture;
class IGLRender
{
public:
    static const unsigned int DISPLAY_MODE_MIN = 0;
    static const unsigned int DISPLAY_MODE_PANO_PLUS_FRONT = DISPLAY_MODE_MIN;
    static const unsigned int DISPLAY_MODE_PANO_PLUS_LEFT = 1;
    static const unsigned int DISPLAY_MODE_PANO_PLUS_RIGHT = 2;
    static const unsigned int DISPLAY_MODE_PANO_PLUS_REAR = 3;
    static const unsigned int DISPLAY_MODE_FRONT = 4;
    static const unsigned int DISPLAY_MODE_LEFT = 5;
    static const unsigned int DISPLAY_MODE_RIGHT = 6;
    static const unsigned int DISPLAY_MODE_REAR = 7;
    static const unsigned int DISPLAY_MODE_MAX = DISPLAY_MODE_REAR;

public:
    virtual ~IGLRender() {}
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
    virtual int start(unsigned int fps) = 0;
    virtual void stop() = 0;

    virtual void draw() = 0;
};

#endif // IGLRENDER_H
