#include "util.h"
#include <opencv/highgui.h>

Util::Util(QObject *parent) :
    QObject(parent)
{
}


void Util::write2File(int channel, IplImage* frame)
{
    static int count = 0;
    char outImageName[16] = {0};
    IplImage* outImage = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);
    // 将原图拷贝过来
    cvCopy(frame,outImage,NULL);

    //设置保存的图片名称和格式
    memset(outImageName, 0, sizeof(outImageName));
    sprintf(outImageName, "test_cam%d_%d.jpg", channel, count++);
    //保存图片
    cvSaveImage(outImageName, outImage, 0);
}
