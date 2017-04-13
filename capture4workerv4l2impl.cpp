#include "capture4workerv4l2impl.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include "settings.h"
#include <sys/ioctl.h>
#if USE_IMX_IPU
#include <linux/ipu.h>
#endif

Capture4WorkerV4l2Impl::Capture4WorkerV4l2Impl(QObject *parent, int videoChannelNum) :
    Capture4WorkerBase(parent, videoChannelNum)
{
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mWidth[i] = 704;
        mHeight[i] = 576;
        mFmt[i] = V4L2_PIX_FMT_UYVY;
        mIPUFd[i] = -1;
        mVideoFd[i] = -1;
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
        mVideoFd[i] = open(devName, O_RDWR/* | O_NONBLOCK*/);
        if (mVideoFd[i] < 0)
        {
            qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                    << " open video failed";
            return -1;
        }

        V4l2::getVideoCap(mVideoFd[i]);
        V4l2::getVideoFmt(mVideoFd[i], &mFmt[i], &mWidth[i], &mHeight[i]);
        //i don't know why
        V4l2::setVideoFmt(mVideoFd[i], mFmt[i], mWidth[i]-2, mHeight[i]-2);
        V4l2::getVideoFmt(mVideoFd[i], &mFmt[i], &mWidth[i], &mHeight[i]);
        V4l2::setFps(mVideoFd[i], 15);
        V4l2::getFps(mVideoFd[i]);

        qDebug() << "Capture4WorkerV4l2Impl::openDevice"
                 << "mem type: " << V4L2_MEMORY_MMAP
                 << "buf count:" << V4L2_BUF_COUNT
                << " width:" << mWidth[i]
                << " height:" << mHeight[i];

        for (unsigned int j = 0; j < V4L2_BUF_COUNT; ++j)
        {
            mV4l2Buf[i][j].width = mWidth[i];
            mV4l2Buf[i][j].height = mHeight[i];
            mV4l2Buf[i][j].fmt = mFmt[i];
        }

        unsigned int frame_size = mWidth[i] * mHeight[i] * 2;
        if (-1 == V4l2::initV4l2Buf(mVideoFd[i], mV4l2Buf[i], V4L2_BUF_COUNT, mMemType, mIPUFd[i], frame_size))
        {
            return -1;
        }
#if USE_IMX_IPU
        mIpuBuf[i].width = mWidth[i];
        mIpuBuf[i].height = mHeight[i];
        mIpuBuf[i].fmt = V4L2_PIX_FMT_BGR24;

        frame_size = mIpuBuf[i].width * mIpuBuf[i].height * 3;
        if (-1 == V4l2::initIpuBuf(mIPUFd[i], &(mIpuBuf[i]), 1, frame_size ))
        {
            return -1;
        }
#endif
        if (-1 == V4l2::startCapture(mVideoFd[i], mV4l2Buf[i], mMemType))
        {
            return -1;
        }
    }

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
                double convert_start = (double)clock();
#endif

#if USE_IMX_IPU
                struct ipu_task task;
                memset(&task, 0, sizeof(struct ipu_task));
                task.input.width  = mWidth[i];
                task.input.height = mHeight[i];
                task.input.crop.pos.x = 0;
                task.input.crop.pos.y = 0;
                task.input.crop.w = mWidth[i];
                task.input.crop.h = mHeight[i];
                task.input.format = mFmt[i];
                task.input.deinterlace.enable = 1;
                task.input.deinterlace.motion = 2;

                task.output.width = mWidth[i];
                task.output.height = mHeight[i];
                task.output.crop.pos.x = 0;
                task.output.crop.pos.y = 0;
                task.output.crop.w = mWidth[i];
                task.output.crop.h = mHeight[i];
                //for colour cast
                //task.output.format = V4L2_PIX_FMT_RGB24;
                task.output.format = V4L2_PIX_FMT_BGR24;

                task.input.paddr = (int)mV4l2Buf[i][buf.index].offset;
                task.output.paddr = (int)mIpuBuf[i].offset;
                if (ioctl(mIPUFd[i], IPU_QUEUE_TASK, &task) < 0) {
                    qDebug() << "Capture1WorkerV4l2Impl::onCapture"
                        << " ipu task failed:" << mIPUFd[i];
                    continue;
                }
                
#else
                int imageSize = mWidth[i]*mHeight[i]*3;
                unsigned char frame_buffer[imageSize];
                Util::uyvy_to_rgb24(mWidth[i], mHeight[i], (unsigned char*)(mV4l2Buf[i][buf.index].start), frame_buffer);
#endif

#if DEBUG_CAPTURE
                convert_time = (int)(clock() - convert_start)/1000;
                qDebug() << "Capture4WorkerV4l2Impl::onCapture"
                         << ", channel:" << i
                         << ", yuv to rgb:" << convert_time;
#endif

#if DATA_TYPE_IPLIMAGE
                IplImage* pIplImage = cvCreateImage(cvSize(mWidth[i], mHeight[i]), IPL_DEPTH_8U, 3);
                if (NULL != pIplImage)
                {
                    memcpy(pIplImage->imageData, frame_buffer, imageSize);
                    flag = flag << 1;
                    image[i] = pIplImage;
                }
#else
                //matImage[i] = new cv::Mat(mHeight[i], mWidth[i], CV_8UC3, frame_buffer);
                //IplImage* pIplImage = cvCreateImage(cvSize(mWidth[i], mHeight[i]), IPL_DEPTH_8U, 3);
#if USE_IMX_IPU                
                //memcpy(pIplImage->imageData, (void*)(mIpuBuf[i].start), mIpuBuf[i].length);
                cv::Mat* matImage = new cv::Mat(mHeight[i], mWidth[i], CV_8UC3, mIpuBuf[i].start);
#else
                //memcpy(pIplImage->imageData, frame_buffer, imageSize);
                cv::Mat* matImage = new cv::Mat(mHeight[i], mWidth[i], CV_8UC3, frame_buffer);
#endif
                //cv::Mat* matImage = new cv::Mat(pIplImage, true);
                if (NULL != matImage)
                {
                    flag = flag << 1;
                    image[i] = matImage;
                }
                //cvReleaseImage(&pIplImage);
#endif
            }
        }

#if DEBUG_CAPTURE
                qDebug() << "Capture4WorkerV4l2Impl::onCapture"
                         << ", read frame:" << (int)(clock() - read_start)/1000;
#endif

        V4l2::v4l2QBuf(mVideoFd[i], &buf);
    }

    //integrity
    if (flag == (1 << mVideoChannelNum))
    {
        surround_image4_t* surroundImage = new surround_image4_t();
        surroundImage->timestamp = timestamp;
        for (int i = 0; i < mVideoChannelNum; ++i)
        {
            surroundImage->image[i] = image[i];
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
#if DATA_TYPE_IPLIMAGE
                cvReleaseImage((IplImage**)image[i]);
#else
                delete (cv::Mat*)image[i];
#endif
            }
        }
    }

#if DEBUG_CAPTURE
    qDebug() << "Capture4WorkerV4l2Impl::onCapture"
             << ", channel:" << mVideoChannelNum
             << ", flag:" << flag
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", yuv to rgb:" << convert_time
             << ", capture:" << (int)(clock()-timestamp)/1000;
#endif
}
