#include "capture1workerv4l2.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <sys/ioctl.h>
#include <ostream>

#define USE_IMX_IPU 0

#if USE_IMX_IPU
#include <linux/ipu.h>
#endif

Capture1WorkerV4l2::Capture1WorkerV4l2() :
    Capture1WorkerBase()
{
    mIPUFd = -1;

#if USE_IMX_IPU
    mMemType = V4L2_MEMORY_USERPTR;
#else
    mMemType = V4L2_MEMORY_MMAP;
#endif

    pthread_mutex_init(&mMutexV4l2, NULL);
    pthread_mutex_init(&mMutexIpu, NULL);
}

Capture1WorkerV4l2::~Capture1WorkerV4l2()
{

}

int Capture1WorkerV4l2::openDevice(unsigned int channel)
{
    mVideoChannel = channel;
#if USE_IMX_IPU
    mIPUFd = open("/dev/mxc_ipu", O_RDWR, 0);
    if (mIPUFd < 0)
    {
        std::cout << "Capture1WorkerV4l2::openDevice"
                << ", open ipu failed"
                << std::endl;
        return -1;
    }
    std::cout << "Capture1WorkerV4l2::openDevice"
            << ", ipu fd:" << mIPUFd
            << std::endl;
#endif

    char devName[16] = {0};
    sprintf(devName, "/dev/video%d", mVideoChannel);
    mVideoFd = open(devName, O_RDWR | O_NONBLOCK);
    if (mVideoFd <= 0)
    {
        return -1;
    }

    unsigned int pixfmt;
    unsigned int width;
    unsigned int height;

    V4l2::getVideoCap(mVideoFd);
    V4l2::getVideoFmt(mVideoFd, &pixfmt, &width, &height);
    //i don't know why
    V4l2::setVideoFmt(mVideoFd, mSink.pixfmt, mSink.width, mSink.height);
    V4l2::getVideoFmt(mVideoFd, &mSink.pixfmt, &mSink.width, &mSink.height);

    V4l2::setFps(mVideoFd, 15);
    V4l2::getFps(mVideoFd);
#if DEBUG_CAPTURE
        std::cout << "Capture1WorkerV4l2::openDevice:" << devName
                << " mem type: " << mMemType
                << " buf count:" << V4L2_BUF_COUNT
                << " in_pixfmt:" << mSink.pixfmt
                << " in_width:" << mSink.width
                << " in_height:" << mSink.height
		        << std::endl;
#endif

    mSource.width = mSink.width;
    mSource.height = mSink.height;

    if (mSink.pixfmt == V4L2_PIX_FMT_UYVY)
    {
        mSink.size = mSink.width * mSink.height * 2;        
    }

    if (mSource.pixfmt == V4L2_PIX_FMT_UYVY)
    {
        mSource.size = mSource.width * mSource.height * 2;        
    }
    else if (mSource.pixfmt == V4L2_PIX_FMT_BGR24)
    {
        mSource.size = mSource.width * mSource.height * 3;
    }

    for (unsigned int i = 0; i < V4L2_BUF_COUNT; ++i)
    {
        mV4l2Buf[i].width = mSink.width;
        mV4l2Buf[i].height = mSink.height;
        mV4l2Buf[i].pixfmt = mSink.pixfmt;
    }

    if (-1 == V4l2::v4l2ReqBuf(mVideoFd, mV4l2Buf, V4L2_BUF_COUNT, mMemType, mIPUFd, mSink.size))
    {
        return -1;
    }

#if USE_IMX_IPU
    if (-1 == IMXIPU::allocIPUBuf(mIPUFd, &mOutIPUBuf, mSource.size))
    {
        return -1;
    }
#endif

#if DEBUG_CAPTURE
        std::cout << "Capture1WorkerV4l2::openDevice:" << devName
                << " initV4l2Buf ok"
		        << std::endl;
#endif

    if (-1 == V4l2::startCapture(mVideoFd, mV4l2Buf, mMemType))
    {
        return -1;
    }

#if DEBUG_CAPTURE
        std::cout << "Capture1WorkerV4l2::openDevice"
                << " startCapture ok"
                << std::endl;
#endif

    return 0;
}

void Capture1WorkerV4l2::closeDevice()
{
    if (mVideoFd > 0)
    {
        V4l2::stoptCapture(mVideoFd);

        close(mVideoFd);
        mVideoFd = -1;
    }
}

void Capture1WorkerV4l2::clearOverstock()
{
    pthread_mutex_lock(&mMutexQueue);
    int size = mSurroundImageQueue.size();
    if (size > 5)
    {
        for (int i = 0; i < size; ++i)
        {
            struct surround_image_t* surroundImage = mSurroundImageQueue.front();
            mSurroundImageQueue.pop();
            if (NULL != surroundImage)
            {
                delete surroundImage;
            }
        }
    }
    pthread_mutex_unlock(&mMutexQueue);
}

bool Capture1WorkerV4l2::isNeedConvert()
{
    return (mSink.pixfmt != mSource.pixfmt
                    || mSink.width != mSource.width
                    || mSink.height != mSource.height);
}

void Capture1WorkerV4l2::run()
{
    clearOverstock();
#if DEBUG_CAPTURE
    clock_t start = clock();
    int size = 0;
    int focus_size = 0;
    double elapsed = 0;
    clock_t read_time = 0;
    clock_t convert_time = 0;
    if (mLastCallTime != 0)
    {
        elapsed = (double)(start - mLastCallTime)/CLOCKS_PER_SEC;
    }
    mLastCallTime = start;
#endif

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
        {
            std::cout << "CaptureWorkerV4l2::onCapture"
                    << ", EINTR"
                    << std::endl;
        }
        return;
    }

    if (0 == r) {
        std::cout << "CaptureWorkerV4l2::onCapture"
                << ", select timeout"
                << std::endl;
        return;
    }

    double read_start = (double)clock();
    struct v4l2_buffer buf;
    if (-1 != V4l2::readFrame(mVideoFd, &buf, mMemType))
    {
        if (buf.index < V4L2_BUF_COUNT)
        {
            if (mFocusSource.pixfmt == mSink.pixfmt)
            {
                surround_image_t* surround_image = new surround_image_t();
                surround_image->timestamp = Util::get_system_milliseconds();
                surround_image->info.pixfmt = mFocusSource.pixfmt;
                surround_image->info.width = mFocusSource.width;
                surround_image->info.height = mFocusSource.height;
                surround_image->info.size = mFocusSource.size;
                //surround_image->data = new unsigned char[mFocusSource.size];
                //memcpy((unsigned char*)surround_image->data, (unsigned char*)mV4l2Buf[i][buf.index].start, mFocusSource.size);
                surround_image->data = mV4l2Buf[buf.index].start;
                pthread_mutex_lock(&mMutexFocusSourceQueue);
                mFocusSourceQueue.push(surround_image);
#if DEBUG_CAPTURE
                focus_size = mFocusSourceQueue.size();
#endif
                pthread_mutex_unlock(&mMutexFocusSourceQueue);   

                if (mEnableCapture)                                  
                {
                    mCaptureFrame4FocusSource.timestamp = surround_image->timestamp;
                    mCaptureFrame4FocusSource.info.pixfmt = surround_image->info.pixfmt;
                    mCaptureFrame4FocusSource.info.width = surround_image->info.width;
                    mCaptureFrame4FocusSource.info.height = surround_image->info.height;
                    mCaptureFrame4FocusSource.info.size = surround_image->info.size;
                    mCaptureFrame4FocusSource.data = surround_image->data;
                }
            }

            if (mSource.pixfmt == mSink.pixfmt)
            {
                surround_image_t* surround_image = new surround_image_t();
                surround_image->timestamp = Util::get_system_milliseconds();
                surround_image->info.pixfmt = mSource.pixfmt;
                surround_image->info.width = mSource.width;
                surround_image->info.height = mSource.height;
                surround_image->info.size = mSource.size;
                //surround_image->data = new unsigned char[mFocusSource.size];
                //memcpy((unsigned char*)surround_image->data, (unsigned char*)mV4l2Buf[i][buf.index].start, mFocusSource.size);
                surround_image->data = mV4l2Buf[buf.index].start;

                pthread_mutex_lock(&mMutexQueue);
                mSurroundImageQueue.push(surround_image);
#if DEBUG_CAPTURE
                size = mSurroundImageQueue.size();
#endif
                pthread_mutex_unlock(&mMutexQueue);
            }
        }
    }

    V4l2::v4l2QueueBuf(mVideoFd, &buf);

#if DEBUG_CAPTURE
    std::cout << "Capture1WorkerV4l2::run"
            << ", thread id:" << getTID()
            << ", channel:" << mVideoChannel
            << ", elapsed to last:" << elapsed
            << ", capture:" << (double)(clock() - start)/CLOCKS_PER_SEC
            << ", size:" << size
            << ", focus_size:" << focus_size
            << std::endl;
#endif
}
