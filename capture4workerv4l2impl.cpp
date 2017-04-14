#include "capture4workerv4l2impl.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include "settings.h"
#include <QDebug>
#include <sys/ioctl.h>
#if USE_IMX_IPU
#include <linux/ipu.h>
#endif

Capture4WorkerV4l2Impl::Capture4WorkerV4l2Impl(QObject *parent, int videoChannelNum) :
    Capture4WorkerBase(parent, videoChannelNum)
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mVideoFd[i] = -1;

        mIPUFd[i] = -1;

        mInWidth[i] = 704;
        mInHeight[i] = 574;
        mInPixfmt[i] = V4L2_PIX_FMT_UYVY;

        mOutWidth[i] = 704;
        mOutHeight[i] = 574;
        mOutPixfmt[i] = V4L2_PIX_FMT_BGR24;
#if USE_IMX_IPU
        mInIPUBuf[i].width = mInWidth[i];
        mInIPUBuf[i].height = mInHeight[i];
        mInIPUBuf[i].pixfmt = mInPixfmt[i];

        mOutIPUBuf[i].width = mOutWidth[i];
        mOutIPUBuf[i].height = mOutHeight[i];
        mOutIPUBuf[i].pixfmt = mOutPixfmt[i];
#endif
    }
#if USE_IMX_IPU
    mMemType = V4L2_MEMORY_USERPTR;
#else
    mMemType = V4L2_MEMORY_MMAP;
#endif
}

int Capture4WorkerV4l2Impl::openDevice()
{
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
#if USE_IMX_IPU
        mIPUFd[i] = open("/dev/mxc_ipu", O_RDWR, 0);
        if (mIPUFd[i] < 0)
        {
            qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                    << " open ipu failed";
            return -1;
        }
        qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                << " ipu fd:" << mIPUFd[i];

#endif

        int video_channel = Settings::getInstant()->mVideoChanel[i];
        char devName[16] = {0};
        sprintf(devName, "/dev/video%d", video_channel);
        mVideoFd[i] = open(devName, O_RDWR | O_NONBLOCK);
        if (mVideoFd[i] < 0)
        {
            qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                    << " open video failed";
            return -1;
        }

        V4l2::getVideoCap(mVideoFd[i]);
        V4l2::getVideoFmt(mVideoFd[i], &mInPixfmt[i], &mInWidth[i], &mInHeight[i]);
        //i don't know why
        V4l2::setVideoFmt(mVideoFd[i], mInPixfmt[i], mInWidth[i]-2, mInHeight[i]-2);
        V4l2::getVideoFmt(mVideoFd[i], &mInPixfmt[i], &mInWidth[i], &mInHeight[i]);
        V4l2::setFps(mVideoFd[i], 15);
        V4l2::getFps(mVideoFd[i]);

#if DEBUG_CAPTURE
        qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                 << "mem type: " << mMemType
                 << "buf count:" << V4L2_BUF_COUNT
                << " width:" << mInWidth[i]
                << " height:" << mInHeight[i];
#endif

        for (unsigned int j = 0; j < V4L2_BUF_COUNT; ++j)
        {
            mV4l2Buf[i][j].width = mInWidth[i];
            mV4l2Buf[i][j].height = mInHeight[i];
            mV4l2Buf[i][j].pixfmt = mInPixfmt[i];
        }

        unsigned int in_frame_size = mInWidth[i] * mInHeight[i] * 2;
        if (-1 == V4l2::v4l2ReqBuf(mVideoFd[i], mV4l2Buf[i], V4L2_BUF_COUNT, mMemType, mIPUFd[i], in_frame_size))
        {
            return -1;
        }

#if USE_IMX_IPU
        unsigned int out_frame_size = mOutIPUBuf[i].width * mOutIPUBuf[i].height * 3;
        if (-1 == IMXIPU::allocIPUBuf(mIPUFd[i], &(mOutIPUBuf[i]),  out_frame_size))
        {
            return -1;
        }
#endif

#if DEBUG_CAPTURE
        qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                 << "initV4l2Buf ok";
#endif

#if USE_IMX_IPU

#endif

        if (-1 == V4l2::startCapture(mVideoFd[i], mV4l2Buf[i], mMemType))
        {
            return -1;
        }
    }

#if DEBUG_CAPTURE
        qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                 << "startCapture ok";
#endif

    return 0;
}

void Capture4WorkerV4l2Impl::closeDevice()
{
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        if (mVideoFd[i] > 0)
        {
            V4l2::stoptCapture(mVideoFd[i]);

            close(mVideoFd[i]);
            mVideoFd[i] = -1;
        }
    }
}

void Capture4WorkerV4l2Impl::onCapture()
{
#if DEBUG_CAPTURE
    double start = (double)clock();
    int size = 0;
    int elapsed = 0;
    int read_time = 0;
    int convert_time = 0;
    if (qAbs(mLastTimestamp) > 0.00001f)
    {
        elapsed = (int)(start - mLastTimestamp)/1000;
    }
    mLastTimestamp = start;
#endif

    void* image[VIDEO_CHANNEL_SIZE] = {NULL};
    unsigned char flag = 1;
    double timestamp = (double)clock();
    for (int i = 0; i < mVideoChannelNum; ++i)
    {
        if (mVideoFd[i] == -1)
        {
            return;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(mVideoFd[i], &fds);

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        int r = select (mVideoFd[i] + 1, &fds, NULL, NULL, &tv);
        if (-1 == r) {
            if (EINTR == errno)
                qDebug() << "Capture1WorkerV4l2Impl::onCapture"
                         << "EINTR";
                return;
        }

        if (0 == r) {
            qDebug() << "Capture1WorkerV4l2Impl::onCapture"
                     << " select timeout";
            return;
        }

#if DEBUG_CAPTURE
        double read_start = (double)clock();
#endif
        struct v4l2_buffer buf;
        if (-1 != V4l2::readFrame(mVideoFd[i], &buf, mMemType))
        {
            if (buf.index < V4L2_BUF_COUNT)
            {
#if DEBUG_CAPTURE
                read_time = (int)(clock()-read_start)/1000;
#endif

#if DEBUG_CAPTURE
                double convert_start = (double)clock();
#endif

#if USE_IMX_IPU
                struct ipu_task task;
                memset(&task, 0, sizeof(struct ipu_task));
                task.input.width  = mInWidth[i];
                task.input.height = mInHeight[i];
                task.input.crop.pos.x = 0;
                task.input.crop.pos.y = 0;
                task.input.crop.w = mInWidth[i];
                task.input.crop.h = mInHeight[i];
                task.input.format = mInPixfmt[i];
                task.input.deinterlace.enable = 1;
                task.input.deinterlace.motion = 2;

                task.output.width = mOutWidth[i];
                task.output.height = mOutHeight[i];
                task.output.crop.pos.x = 0;
                task.output.crop.pos.y = 0;
                task.output.crop.w = mOutWidth[i];
                task.output.crop.h = mOutHeight[i];
                //for colour cast
                //task.output.format = V4L2_PIX_FMT_RGB24;
                task.output.format = mOutPixfmt[i];

                task.input.paddr = (int)mV4l2Buf[i][buf.index].offset;
                task.output.paddr = (int)mOutIPUBuf[i].offset;
                if (ioctl(mIPUFd[i], IPU_QUEUE_TASK, &task) < 0) {
                    qDebug() << "Capture1WorkerV4l2Impl::onCapture"
                        << " ipu task failed:" << mIPUFd[i];
                    continue;
                }
                
#else
                int imageSize = mOutWidth[i]*mOutHeight[i]*3;
                unsigned char frame_buffer[imageSize];
                Util::uyvy_to_rgb24(mInWidth[i], mInHeight[i], (unsigned char*)(mV4l2Buf[i][buf.index].start), frame_buffer);
#endif

#if DEBUG_CAPTURE
                convert_time = (int)(clock() - convert_start)/1000;
                qDebug() << "Capture4WorkerV4l2Impl::onCapture"
                         << " channel:" << i
                         <<", read_time:" << read_time
                         << ", yuv to rgb:" << convert_time;
#endif

#if USE_IMX_IPU                
                cv::Mat* matImage = new cv::Mat(mOutHeight[i], mOutWidth[i], CV_8UC3, mOutIPUBuf[i].start);
#else
                cv::Mat* matImage = new cv::Mat(mOutHeight[i], mOutWidth[i], CV_8UC3, frame_buffer);
#endif
                if (NULL != matImage)
                {
                    flag = flag << 1;
                    image[i] = matImage;
                }
            }
        }

        V4l2::v4l2QueueBuf(mVideoFd[i], &buf);
    }

    //integrity
    if (flag == (1 << mVideoChannelNum))
    {
        surround_images_t* surroundImage = new surround_images_t();
        surroundImage->timestamp = timestamp;
        for (int i = 0; i < mVideoChannelNum; ++i)
        {
            surroundImage->frame[i].data = image[i];
            surroundImage->frame[i].width = mOutWidth[i];
            surroundImage->frame[i].height = mOutHeight[i];
            surroundImage->frame[i].pixfmt = mOutPixfmt[i];
        }

        mMutexQueue.lock();
        mSurroundImageQueue.append(surroundImage);
#if DEBUG_CAPTURE
        size = mSurroundImageQueue.size();
#endif
        mMutexQueue.unlock();
    }
    else
    {
        for (int i = 0; i < mVideoChannelNum; ++i)
        {
            if (NULL != image[i])
            {
                delete (cv::Mat*)image[i];
            }
        }
    }

#if DEBUG_CAPTURE
    qDebug() << "Capture4WorkerV4l2Impl::onCapture"
             << "  channel:" << mVideoChannelNum
             << ", flag:" << flag
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (int)(clock()-timestamp)/1000;
#endif
}
