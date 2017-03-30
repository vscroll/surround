#include "capture1workerv4l2impl.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"

Capture1WorkerV4l2Impl::Capture1WorkerV4l2Impl(QObject *parent, int videoChannel) :
    Capture1WorkerBase(parent, videoChannel),
    mWidth(704),
    mHeight(576),
    mV4l2Buf(NULL),
    mVideoFd(-1)
{
}

void Capture1WorkerV4l2Impl::openDevice()
{
    char devName[16] = {0};
    sprintf(devName, "/dev/video%d", mVideoChannel);
    mVideoFd = open(devName, O_RDWR/* | O_NONBLOCK*/);
    if (mVideoFd <= 0)
    {
        return;
    }

    V4l2::getVideoCap(mVideoFd);
    V4l2::setVideoFmt(mVideoFd, mWidth, mHeight);
    V4l2::getVideoFmt(mVideoFd, &mWidth, &mHeight);
    V4l2::getFps(mVideoFd);
    if (-1 == V4l2::initV4l2Buf(mVideoFd, &mV4l2Buf))
    {
        return;
    }

    if (-1 == V4l2::startCapture(mVideoFd))
    {
        return;
    }
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
        qDebug() << "Capture1WorkerV4l2Impl::onCapture"
                 << " select timeout";
        return;
    }

    double start = (double)clock();
    int size = 0;
    int elapsed = 0;
    int convert_time = 0;
    if (qAbs(mLastTimestamp) > 0.00001f)
    {
        elapsed = (int)(start - mLastTimestamp)/1000;
    }
    mLastTimestamp = start;

    int imageSize = mWidth*mHeight*3;
    double timestamp = (double)clock();

    struct v4l2_buffer buf;
    if (-1 != V4l2::readFrame(mVideoFd, &buf))
    {
        if (buf.index < V4l2::V4L2_BUF_COUNT)
        {
            unsigned char frame_buffer[imageSize];
            mMutexCapture.lock();
#if DEBUG
            double convert_start = (double)clock();
#endif
            Util::yuyv_to_rgb24(mWidth, mHeight, (unsigned char*)(mV4l2Buf[buf.index].start), frame_buffer);
            //memset((unsigned char*)(mV4l2Buf[buf.index].start), 0, mV4l2Buf[buf.index].length);
#if DEBUG
            convert_time = (int)(clock() - convert_start)/1000;
#endif
            mMutexCapture.unlock();

            cv::Mat* image = new cv::Mat(mHeight, mWidth, CV_8UC3, frame_buffer);
            if (NULL != image)
            {
                //memcpy(image->data, frame_buffer, imageSize);
                //write2File(pIplImage);
                //cvReleaseImage(&pIplImage);

                surround_image1_t* surroundImage = new surround_image1_t();
                surroundImage->timestamp = timestamp;
                surroundImage->image = image;
                mMutexQueue.lock();
                mSurroundImageQueue.append(surroundImage);
                size = mSurroundImageQueue.size();
                mMutexQueue.unlock();
            }
        }
    }

    V4l2::v4l2QBuf(mVideoFd, &buf);

#if DEBUG
    qDebug() << "Capture1WorkerV4l2Impl::onCapture"
             << ", channel:" << mVideoChannel
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", yuv to rgb:" << convert_time
             << ", capture:" << (int)(clock()-timestamp)/1000;
#endif
}
