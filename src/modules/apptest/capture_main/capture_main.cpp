// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "common.h"
#include "ICapture.h"
#include "captureimpl.h"
#include "imageshm.h"
#include <linux/input.h>

class SideImageSHMWriteWorker : public Thread
{
public:
    SideImageSHMWriteWorker(ICapture* capture);
    virtual ~SideImageSHMWriteWorker();

public:
    virtual void run();

private:
    ICapture* mCapture;
    ImageSHM* mImageSHM;
};

SideImageSHMWriteWorker::SideImageSHMWriteWorker(ICapture* capture)
{
    mCapture = capture;
    mImageSHM = new ImageSHM();
    mImageSHM->create((key_t)SHM_SIDE_ID, SHM_SIDE_SIZE);
}

SideImageSHMWriteWorker::~SideImageSHMWriteWorker()
{
    if (NULL != mImageSHM)
    {
        mImageSHM->destroy();
        delete mImageSHM;
        mImageSHM = NULL;
    }
}

void SideImageSHMWriteWorker::run()
{
    if (NULL == mCapture
        || NULL == mImageSHM)
    {
        return;
    }

    surround_image_t* sideImage = mCapture->popOneFrame4FocusSource();
    if (NULL != sideImage)
    {
		struct image_shm_header_t header = {};
		header.width = sideImage->info.width;
		header.height = sideImage->info.height;
		header.pixfmt = sideImage->info.pixfmt;
		header.size = sideImage->info.size;
		header.timestamp = sideImage->timestamp;
        unsigned char* frame = (unsigned char*)sideImage->data;
#if 0
        clock_t start = clock();
#endif
        if (NULL != frame)
        {
            mImageSHM->writeImage(&header, frame, header.size);
        }
#if 0
        std::cout << " side shm write time: " << (double)(clock()-start)/CLOCKS_PER_SEC
                << " width:" << header.width
                << " height:" << header.height
                << " size:" << header.size
                << " timestamp:" << header.timestamp
                << std::endl;
#endif
    }
}

int main (int argc, char **argv)
{
    ICapture* capture = new CaptureImpl();
    unsigned int channel[VIDEO_CHANNEL_SIZE] = {4,2,3,5};
    struct cap_sink_t sink[VIDEO_CHANNEL_SIZE];
    struct cap_src_t source[VIDEO_CHANNEL_SIZE];
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        sink[i].pixfmt = PIX_FMT_UYVY;
        sink[i].width = 704;
        sink[i].height = 574;
        sink[i].size = sink[i].width*sink[i].height*2;
        sink[i].crop_x = 0;
        sink[i].crop_y = 0;
        sink[i].crop_w = 704;
        sink[i].crop_h = 574;

        source[i].pixfmt = PIX_FMT_UYVY;
        source[i].width = 704;
        source[i].height = 574;
        source[i].size = source[i].width*source[i].height*2;
    }

    capture->setCapCapacity(sink, source, VIDEO_CHANNEL_SIZE);
    int focusChannelIndex = VIDEO_CHANNEL_FRONT;
    struct cap_src_t focusSource;
    focusSource.pixfmt = sink[0].pixfmt;
    focusSource.width = sink[0].width;
    focusSource.height = sink[0].height;
    focusSource.size = sink[0].size;
    capture->setFocusSource(focusChannelIndex, &focusSource);
    capture->openDevice(channel, VIDEO_CHANNEL_SIZE);
    capture->start(VIDEO_FPS_15);

    SideImageSHMWriteWorker* sideImageSHMWriteWorker = new SideImageSHMWriteWorker(capture);
    sideImageSHMWriteWorker->start(VIDEO_FPS_15);

    int eventFd = open("/dev/input/event0", O_RDONLY);
    struct input_event event[64] = {0};

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(eventFd, &fds);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while (true)
    {
        if (eventFd > 0)
        {
            int r = select (eventFd + 1, &fds, NULL, NULL, &tv);
            if (-1 == r) {
                usleep(100);
            }

            if (0 == r) {
                usleep(100);
            }

            int rd = read(eventFd, event, sizeof(struct input_event) * 64);
            if (rd >= (int) sizeof(struct input_event))
            {
                for (int i = 0; i < rd / sizeof(struct input_event); i++)
                {
                    if (event[i].type == EV_KEY
                            && event[i].code == BTN_TOUCH
                            && event[i].value == 0)
                    {
                        focusChannelIndex++;
                        if (focusChannelIndex >= VIDEO_CHANNEL_SIZE)
                        {
                            focusChannelIndex = VIDEO_CHANNEL_FRONT;
                        }
                        capture->setFocusSource(focusChannelIndex, &focusSource);
                        break;
                    }
                }
            }
        }

        usleep(100);
    }

    sideImageSHMWriteWorker->stop();
    delete sideImageSHMWriteWorker;
    sideImageSHMWriteWorker = NULL;

    capture->closeDevice();
    capture->stop();
    delete capture;
    capture = NULL;

    return 0;
}
