// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common.h"
#include "IPanoImage.h"
#include "panoimageimpl.h"
#include "util.h"
#include "thread.h"
#include <opencv/cv.h>

class DataSchWorker : public Thread
{
public:
    DataSchWorker();
    virtual ~DataSchWorker();

public:
    virtual void run();

private:
    IPanoImage* mPanoImage;
};

DataSchWorker::DataSchWorker()
{
    mPanoImage = new PanoImageImpl();
    mPanoImage->init(704, 574, PIX_FMT_BGR24,
                424, 600, PIX_FMT_BGR24,
                "/home/root/ckt-demo/PanoConfig.bin", true);
    mPanoImage->start(VIDEO_FPS_15);
}

DataSchWorker::~DataSchWorker()
{

}

void DataSchWorker::run()
{
    long timestamp = Util::get_system_milliseconds();
    surround_images_t* surroundImages = new surround_images_t();
    surroundImages->timestamp = timestamp;
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        surroundImages->frame[i].timestamp = timestamp;
        surroundImages->frame[i].info.width = 704;
        surroundImages->frame[i].info.height = 574;
        surroundImages->frame[i].info.pixfmt = PIX_FMT_BGR24;
        surroundImages->frame[i].info.size = surroundImages->frame[i].info.width*surroundImages->frame[i].info.height*3;

        unsigned char frame_buffer[surroundImages->frame[i].info.size] = {0};
        cv::Mat* image = new cv::Mat(surroundImages->frame[i].info.height, surroundImages->frame[i].info.width, CV_8UC3, frame_buffer);
        surroundImages->frame[i].data = image;
    }
    mPanoImage->queueImages(surroundImages);
}

int main (int argc, char **argv)
{
    DataSchWorker* dataSchWorker = new DataSchWorker();
    dataSchWorker->start(VIDEO_FPS_15);

    while (true)
    {
        sleep(100);
    }

    return 0;
}
