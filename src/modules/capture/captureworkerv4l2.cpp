#include "captureworkerv4l2.h"
#include "util.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <iostream>
#include <opencv/cv.h>

#define USE_IMX_IPU 0

#if USE_IMX_IPU
#include <linux/ipu.h>
#endif

CaptureWorkerV4l2::CaptureWorkerV4l2() :
    CaptureWorkerBase()
{
    mIPUFd = -1;

#if USE_IMX_IPU
    mMemType = V4L2_MEMORY_USERPTR;
#else
    mMemType = V4L2_MEMORY_MMAP;
#endif
}

CaptureWorkerV4l2::~CaptureWorkerV4l2()
{
}

int CaptureWorkerV4l2::openDevice(unsigned int channel[], unsigned int channelNum)
{
#if USE_IMX_IPU
    mIPUFd = open("/dev/mxc_ipu", O_RDWR, 0);
    if (mIPUFd < 0)
    {
        std::cout << "CaptureWorkerV4l2::openDevice"
                << " open ipu failed"
                << std::endl;
        return -1;
    }
    std::cout << "CaptureWorkerV4l2::openDevice"
            << " ipu fd:" << mIPUFd
            << std::endl;

#endif

    unsigned int pixfmt;
    unsigned int width;
    unsigned int height;
    mVideoChannelNum = channelNum <= VIDEO_CHANNEL_SIZE ? channelNum: VIDEO_CHANNEL_SIZE;
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
        mChannel[i] = channel[i];
        char devName[16] = {0};
        sprintf(devName, "/dev/video%d", mChannel[i]);
        mVideoFd[i] = open(devName, O_RDWR | O_NONBLOCK);
        if (mVideoFd[i] < 0)
        {
            std::cout << "CaptureWorkerV4l2::openDevice:" << devName
                << " open video failed"
                << std::endl;
            return -1;
        }

        V4l2::getVideoCap(mVideoFd[i]);
        V4l2::getVideoFmt(mVideoFd[i], &pixfmt, &width, &height);

        //V4l2::setVideoFmt(mVideoFd[i], mSink[i].pixfmt, width-2, height-2);
        //V4l2::getVideoFmt(mVideoFd[i], &mSink[i].pixfmt, &mSink[i].width, &mSink[i].height);
        //V4l2::setFps(mVideoFd[i], 15);
        //V4l2::getFps(mVideoFd[i]);

        if (V4l2::setVideoFmt(mVideoFd[i], mSink[i].pixfmt, mSink[i].width, mSink[i].height) < 0)
        {
            return -1;
        }

#if DEBUG_CAPTURE
        std::cout << "CaptureWorkerV4l2::openDevice:" << devName
                << " mem type: " << mMemType
                << " buf count:" << V4L2_BUF_COUNT
                << " in_pixfmt:" << mSink[i].pixfmt
                << " in_width:" << mSink[i].width
                << " in_height:" << mSink[i].height
                << " in_size:" << mSink[i].size
		        << std::endl;
#endif

        for (unsigned int j = 0; j < V4L2_BUF_COUNT; ++j)
        {
            mV4l2Buf[i][j].width = mSink[i].width;
            mV4l2Buf[i][j].height = mSink[i].height;
            mV4l2Buf[i][j].pixfmt = mSink[i].pixfmt;
        }

        if (-1 == V4l2::v4l2ReqBuf(mVideoFd[i], mV4l2Buf[i], V4L2_BUF_COUNT, mMemType, mIPUFd, mSink[i].size))
        {
            return -1;
        }

#if USE_IMX_IPU
        if (-1 == IMXIPU::allocIPUBuf(mIPUFd, &(mOutIPUBuf[i]), mSource[i].size))
        {
            return -1;
        }
#endif

#if DEBUG_CAPTURE
        std::cout << "CaptureWorkerV4l2::openDevice:" << devName
                << " initV4l2Buf ok"
		        << std::endl;
#endif

#if USE_IMX_IPU

#endif

        if (-1 == V4l2::startCapture(mVideoFd[i], mV4l2Buf[i], V4L2_BUF_COUNT, mMemType))
        {
            return -1;
        }
    }

#if DEBUG_CAPTURE
        std::cout << "CaptureWorkerV4l2::openDevice"
                << " startCapture ok"
                << std::endl;
#endif

    return 0;
}

void CaptureWorkerV4l2::closeDevice()
{
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
        if (mVideoFd[i] > 0)
        {
            V4l2::stoptCapture(mVideoFd[i]);

            close(mVideoFd[i]);
            mVideoFd[i] = -1;
        }
    }
}

void CaptureWorkerV4l2::run()
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

    void* image[VIDEO_CHANNEL_SIZE] = {NULL};
    unsigned char flag = 1;
    long timestamp[VIDEO_CHANNEL_SIZE] = {0};
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
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
                std::cout << "CaptureWorkerV4l2::onCapture"
                        << ", EINTR"
                        << std::endl;
                return;
        }

        if (0 == r) {
            std::cout << "CaptureWorkerV4l2::onCapture"
                    << ", select timeout"
                    << std::endl;
            return;
        }

        struct v4l2_buffer buf;
        if (-1 != V4l2::readFrame(mVideoFd[i], &buf, mMemType))
        {
            if (buf.index < V4L2_BUF_COUNT)
            {
                timestamp[i] = Util::get_system_milliseconds();
                if (mFocusChannelIndex == i
                    && !isNeedConvert(&mSink[i], &mFocusSource))
                {
                    surround_image_t* surround_image = new surround_image_t();
                    surround_image->timestamp = timestamp[i];
                    surround_image->info.pixfmt = mFocusSource.pixfmt;
                    surround_image->info.width = mFocusSource.width;
                    surround_image->info.height = mFocusSource.height;
                    surround_image->info.size = mFocusSource.size;
                    //surround_image->data = new unsigned char[mFocusSource.size];
                    //memcpy((unsigned char*)surround_image->data, (unsigned char*)mV4l2Buf[i][buf.index].start, mFocusSource.size);
                    surround_image->data = mV4l2Buf[i][buf.index].start;
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

                // call IPU to convert
                if (isNeedConvert(&mSink[i], &mSource[i]))
                {
#if 0
#if DEBUG_CAPTURE
                    clock_t convert_start = clock();
#endif

#if USE_IMX_IPU
                    struct ipu_task task;
                    memset(&task, 0, sizeof(struct ipu_task));
                    task.input.width  = mSink[i].width;
                    task.input.height = mSink[i].height;
                    task.input.crop.pos.x = mSink[i].crop_x;
                    task.input.crop.pos.y = mSink[i].crop_y;
                    task.input.crop.w = mSink[i].crop_w;
                    task.input.crop.h = mSink[i].crop_h;
                    task.input.format = mSink[i].pixfmt;
                    task.input.deinterlace.enable = 1;
                    task.input.deinterlace.motion = 2;

                    task.output.width = mSource[i].width;
                    task.output.height = mSource[i].height;
                    task.output.crop.pos.x = 0;
                    task.output.crop.pos.y = 0;
                    task.output.crop.w = mSource[i].width;
                    task.output.crop.h = mSource[i].height;
                    //for colour cast
                    //task.output.format = V4L2_PIX_FMT_RGB24;
                    task.output.format = mSource[i].pixfmt;

                    task.input.paddr = (int)mV4l2Buf[i][buf.index].offset;
                    task.output.paddr = (int)mOutIPUBuf[i].offset;
                    if (ioctl(mIPUFd, IPU_QUEUE_TASK, &task) < 0) {
                        std::cout << "CaptureWorkerV4l2::onCapture"
                                << ", ipu task failed:" << mIPUFd
			                    << std::endl;
                        continue;
                    }
                
#else
                    unsigned char frame_buffer[mSource[i].size];
                    Util::uyvy_to_rgb24(mSink[i].width, mSink[i].height, (unsigned char*)(mV4l2Buf[i][buf.index].start), frame_buffer);
#endif

#if DEBUG_CAPTURE
/*
                    convert_time = (clock() - convert_start)/CLOCKS_PER_SEC;
                    std::cout << "CaptureWorkerV4l2::onCapture"
                            << ", channel:" << i
                            << ", yuv to rgb:" << convert_time
			                << std::endl;
*/
#endif

#if USE_IMX_IPU                
                    cv::Mat* matImage = new cv::Mat(mSource[i].height, mSource[i].width, CV_8UC3, mOutIPUBuf[i].start);
#else
                    cv::Mat* matImage = new cv::Mat(mSource[i].height, mSource[i].width, CV_8UC3, frame_buffer);
#endif
                    if (NULL != matImage)
                    {
                        flag = flag << 1;
                        image[i] = matImage;
                    }
#endif
                }
                else
                {
                    flag = flag << 1;
                    //image[i] = new unsigned char[mSource[i].size];
                    //memcpy(image[i], (unsigned char*)mV4l2Buf[i][buf.index].start, mSource[i].size);
                    image[i] = mV4l2Buf[i][buf.index].start;
                }
            }
        }

        V4l2::v4l2QueueBuf(mVideoFd[i], &buf);
    }

    //integrity
    if (flag == (1 << mVideoChannelNum))
    {
        for (unsigned int i = 0; i < mVideoChannelNum; ++i)
        {
            surround_image_t* surroundImage = new surround_image_t();
            surroundImage->timestamp = timestamp[i];
            surroundImage->info.width = mSource[i].width;
            surroundImage->info.height = mSource[i].height;
            surroundImage->info.pixfmt = mSource[i].pixfmt;
            surroundImage->info.size = mSource[i].size;
            surroundImage->data = image[i];

            pthread_mutex_lock(&mMutexQueue[i]);
            mSurroundImageQueue[i].push(surroundImage);
#if DEBUG_CAPTURE
            size = mSurroundImageQueue[i].size();
#endif
            pthread_mutex_unlock(&mMutexQueue[i]);
        }

	    if (mRealFrameCount == 0)
	    {
	        mStartStatTime = clock();
	    }

	    mRealFrameCount++;
        mStatDuration = (clock() - mStartStatTime)/CLOCKS_PER_SEC;
        if (mStatDuration < 1)
        {
            mStatDuration = 1;
        }

	    if (mStatDuration > 5*60)
        {
	        mRealFrameCount = 0;
        }

        mRealFPS = mRealFrameCount/mStatDuration;

    }
    else
    {
#if 0
        for (unsigned int i = 0; i < mVideoChannelNum; ++i)
        {
            if (NULL != image[i]
                && isNeedConvert(mSink[i], mSource[i]))
            {
                delete (cv::Mat*)image[i];
            }
        }
#endif
    }
#if DEBUG_CAPTURE
    std::cout << "CaptureWorkerV4l2::run"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed
            << ", capture:" << (double)(clock() - start)/CLOCKS_PER_SEC
            << ", size:" << size
            << ", focus_size:" << focus_size
            << ", fps:" << mRealFPS
            << std::endl;
#endif
}
