#include "capture1workerv4l2impl.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <sys/ioctl.h>
#include <ostream>
#if USE_IMX_IPU
#include <linux/ipu.h>
#endif

Capture1WorkerV4l2Impl::Capture1WorkerV4l2Impl() :
    Capture1WorkerBase()
{
    mVideoFd = -1;

    mIPUFd = -1;

    mInWidth = 704;
    mInHeight = 574;
    mInPixfmt = V4L2_PIX_FMT_UYVY;

    mOutWidth = 704;
    mOutHeight = 574;
    mOutPixfmt = V4L2_PIX_FMT_BGR24;
#if USE_IMX_IPU
    mInIPUBuf.width = mInWidth;
    mInIPUBuf.height = mInHeight;
    mInIPUBuf.pixfmt = mInPixfmt;

    mOutIPUBuf.width = mOutWidth;
    mOutIPUBuf.height = mOutHeight;
    mOutIPUBuf.pixfmt = mOutPixfmt;
#endif

#if USE_IMX_IPU
    mMemType = V4L2_MEMORY_USERPTR;
#else
    mMemType = V4L2_MEMORY_MMAP;
#endif

    pthread_mutex_init(&mMutexV4l2, NULL);
    pthread_mutex_init(&mMutexIpu, NULL);
}

Capture1WorkerV4l2Impl::~Capture1WorkerV4l2Impl()
{

}

int Capture1WorkerV4l2Impl::openDevice(unsigned int channel)
{
    mVideoChannel = channel;
    if (Capture1WorkerBase::openDevice(mVideoChannel) <= 0)
    {
        return -1;
    }

#if USE_IMX_IPU
    mIPUFd = open("/dev/mxc_ipu", O_RDWR, 0);
    if (mIPUFd < 0)
    {
        std::cout << "Capture1WorkerV4l2Impl::openDevice"
            << ", open ipu failed";
        return -1;
    }
    std::cout << "Capture1WorkerV4l2Impl::openDevice"
        << ", ipu fd:" << mIPUFd;
#endif

    char devName[16] = {0};
    sprintf(devName, "/dev/video%d", mVideoChannel);
    mVideoFd = open(devName, O_RDWR | O_NONBLOCK);
    if (mVideoFd <= 0)
    {
        return -1;
    }

    V4l2::getVideoCap(mVideoFd);
    V4l2::getVideoFmt(mVideoFd, &mInPixfmt, &mInWidth, &mInHeight);
    V4l2::setVideoFmt(mVideoFd, mInPixfmt, mInWidth-2, mInHeight-2);
    V4l2::getVideoFmt(mVideoFd, &mInPixfmt, &mInWidth, &mInHeight);
    V4l2::getFps(mVideoFd);

    unsigned int in_frame_size = mInWidth * mInHeight * 2;
    if (-1 == V4l2::v4l2ReqBuf(mVideoFd, mV4l2Buf, V4L2_BUF_COUNT, mMemType, mIPUFd, in_frame_size))
    {
        return -1;
    }

#if USE_IMX_IPU
    unsigned int out_frame_size = mOutWidth * mOutHeight * 3;
    if (-1 == IMXIPU::allocIPUBuf(mIPUFd, &mOutIPUBuf,  out_frame_size))
    {
        return -1;
    }
#endif

    if (-1 == V4l2::startCapture(mVideoFd, mV4l2Buf, mMemType))
    {
        return -1;
    }

    return 0;
}

void Capture1WorkerV4l2Impl::closeDevice()
{
    if (mVideoFd > 0)
    {
        V4l2::stoptCapture(mVideoFd);

        close(mVideoFd);
        mVideoFd = -1;
    }
}

void Capture1WorkerV4l2Impl::onCapture()
{
    if (mVideoFd == -1)
    {
        return;
    }

    fd_set fds;
    FD_ZERO (&fds);
    FD_SET (mVideoFd, &fds);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    int r = select (mVideoFd + 1, &fds, NULL, NULL, &tv);
    if (-1 == r) {
        if (EINTR == errno)
            return;
    }

    if (0 == r) {
        std::cout << "Capture1WorkerV4l2Impl::onCapture"
                 << ", select timeout";
        return;
    }

    double start = (double)clock();
#if DEBUG_CAPTURE
    int size = 0;
    double elapsed = 0;
    double read_time = 0;
    double convert_time = 0;
    if (mLastTimestamp > 0.00001f)
    {
        elapsed = (int)(start - mLastTimestamp)/CLOCKS_PER_SEC;
    }
#endif
    mLastTimestamp = start;

    double read_start = (double)clock();
    struct v4l2_buffer buf;
    if (-1 != V4l2::readFrame(mVideoFd, &buf, mMemType))
    {
        if (buf.index < V4L2_BUF_COUNT)
        {
#if DEBUG_CAPTURE
            read_time = (clock()-read_start)/CLOCKS_PER_SEC;
#endif

#if DEBUG_CAPTURE
            double convert_start = (double)clock();
#endif

#if USE_IMX_IPU
            struct ipu_task task;
            memset(&task, 0, sizeof(struct ipu_task));
            task.input.width  = mInWidth;
            task.input.height = mInHeight;
            task.input.crop.pos.x = 0;
            task.input.crop.pos.y = 0;
            task.input.crop.w = mInWidth;
            task.input.crop.h = mInHeight;
            task.input.format = mInPixfmt;
            task.input.deinterlace.enable = 1;
            task.input.deinterlace.motion = 2;

            task.output.width = mOutWidth;
            task.output.height = mOutHeight;
            task.output.crop.pos.x = 0;
            task.output.crop.pos.y = 0;
            task.output.crop.w = mOutWidth;
            task.output.crop.h = mOutHeight;
            //for colour cast
            //task.output.format = V4L2_PIX_FMT_RGB24;
            task.output.format = mOutPixfmt;

            pthread_mutex_lock(&mMutexV4l2);
            task.input.paddr = (int)mV4l2Buf[buf.index].offset;
            pthread_mutex_unlock(&mMutexV4l2);

            pthread_mutex_lock(&mMutexIpu);
            task.output.paddr = (int)mOutIPUBuf.offset;
            pthread_mutex_unlock(&mMutexIpu);

            if (ioctl(mIPUFd, IPU_QUEUE_TASK, &task) < 0) {
                std::cout << "Capture1WorkerV4l2Impl::onCapture"
                    << ", ipu task failed:" << mIPUFd;
                return;
            }
#else
            int imageSize = mOutWidth*mOutHeight*3;
            unsigned char frame_buffer[imageSize];
            pthread_mutex_lock(&mMutexV4l2);
            unsigned char* buffer =  (unsigned char*)(mV4l2Buf[buf.index].start);
            pthread_mutex_unlock(&mMutexV4l2);
            Util::uyvy_to_rgb24(mInWidth, mInHeight, buffer, frame_buffer);
#endif
#if DEBUG_CAPTURE
            convert_time = (clock() - convert_start)/CLOCKS_PER_SEC;
#endif

#if USE_IMX_IPU
            pthread_mutex_lock(&mMutexIpu);
            cv::Mat* image = new cv::Mat(mOutHeight, mOutWidth, CV_8UC3, mOutIPUBuf.start);
            pthread_mutex_unlock(&mMutexIpu);
#else
            cv::Mat* image = new cv::Mat(mOutHeight, mOutWidth, CV_8UC3, frame_buffer);
#endif
            if (NULL != image)
            {
                surround_image_t* surroundImage = new surround_image_t();
                surroundImage->timestamp = read_start;
                surroundImage->frame.data = image;
                pthread_mutex_lock(&mMutexQueue);
                mSurroundImageQueue.push(surroundImage);
#if DEBUG_CAPTURE
                size = mSurroundImageQueue.size();
#endif
                pthread_mutex_unlock(&mMutexQueue);
            }
        }
    }

    V4l2::v4l2QueueBuf(mVideoFd, &buf);

#if DEBUG_CAPTURE
    std::cout << "Capture1WorkerV4l2Impl::onCapture"
             << ", channel:" << mVideoChannel
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", read time:" << read_time
             << ", yuv to rgb:" << convert_time
             << ", capture:" << (clock()-read_start)/CLOCKS_PER_SEC
	     << std::endl;
#endif
}
