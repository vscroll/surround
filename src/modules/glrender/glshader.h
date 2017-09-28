#ifndef GLSHADERR_H
#define GLSHADERR_H

#include "esUtil.h"
#include <string>
#include <opencv/cv.h>

class GLShader
{
public:
    GLShader(ESContext* context, const std::string programBinaryFile);
    virtual ~GLShader();

    virtual const char* getVertShader() = 0;
    virtual const char* getFragShader() = 0;
    virtual int initConfig();
    virtual int initProgram();
    virtual void initVertex() = 0;
    virtual void initTexture() = 0;
    virtual void draw() = 0;
    virtual void shutdown();

    virtual void updateFocusChannel() = 0;
    virtual void updatePanoramaView() = 0;

protected:
    void loadLut(int index);

    void getTexImageParam(unsigned int v4l2Pixfmt, GLenum* internalFormat, GLenum* format, GLenum* type);

    static const int PANORAMA_WIDTH = 424;
    static const int PANORAMA_HEIGHT = 600;

    static const int PANORAMA_VIEW_BIRD = 0;
    static const int PANORAMA_VIEW_REAR = 1;
    static const int PANORAMA_VIEW_NUM = 16;

private:
    GLuint LoadProgram(unsigned char *buf, int length);

protected:
    void checkGlError(const char* op);

protected:
    ESContext* mESContext;
    std::string mProgramBinaryFile;
    GLuint mProgramObject;

    unsigned int mFocusChannelIndex;
    unsigned int mPanoramaView;

    cv::Mat mLutAll;
};

#endif // GLSHADERR_H
