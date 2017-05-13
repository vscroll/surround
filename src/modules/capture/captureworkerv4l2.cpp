#include "captureworkerv4l2.h"
#include "util.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <iostream>
#include <opencv/cv.h>
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
            std::cout << "CaptureWorkerV4l2::openDevice"
                << " open video failed"
                << std::endl;
            return -1;
        }

        V4l2::getVideoCap(mVideoFd[i]);
        V4l2::getVideoFmt(mVideoFd[i], &pixfmt, &width, &height);
        //i don't know why
        V4l2::setVideoFmt(mVideoFd[i], mSink[i].pixfmt, width-2, height-2);
        V4l2::getVideoFmt(mVideoFd[i], &mSink[i].pixfmt, &mSink[i].width, &mSink[i].height);

        V4l2::setFps(mVideoFd[i], 15);
        V4l2::getFps(mVideoFd[i]);
#if DEBUG_CAPTURE
        std::cout << "CaptureWorkerV4l2::openDevice"
                << " mem type: " << mMemType
                << " buf count:" << V4L2_BUF_COUNT
                << " in_pixfmt:" << mSink[i].pixfmt
                << " in_width:" << mSink[i].width
                << " in_height:" << mSink[i].height
		<< std::endl;
#endif

	    mSource[i].width = mSink[i].width;
	    mSource[i].height = mSink[i].height;

	    if (mSink[i].pixfmt == PIX_FMT_UYVY)
	    {
            mSink[i].size = mSink[i].width * mSink[i].height * 2;        
	    }

	    if (mSource[i].pixfmt == PIX_FMT_UYVY)
	    {
	        mSource[i].size = mSource[i].width * mSource[i].height * 2;        
	    }
	    else if (mSource[i].pixfmt == PIX_FMT_BGR24)
	    {
	        mSource[i].size = mSource[i].width * mSource[i].height * 3;
	    }

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
        std::cout << "CaptureWorkerV4l2::openDevice"
                << " initV4l2Buf ok"
		        << std::endl;
#endif

#if USE_IMX_IPU

#endif

        if (-1 == V4l2::startCapture(mVideoFd[i], mV4l2Buf[i], mMemType))
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

void CaptureWorkerV4l2::clearOverstock()
{
    pthread_mutex_lock(&mMutexQueue);
    int size = mSurroundImagesQueue.size();
    if (size > 5)
    {
        for (int i = 0; i < size; ++i)
        {
            struct surround_images_t* surroundImage = mSurroundImagesQueue.front();
            mSurroundImagesQueue.pop();
            if (NULL != surroundImage)
            {
                for (unsigned int i = 0; i < mVideoChannelNum; ++i)
                {
                    delete (cv::Mat*)(surroundImage->frame[i].data);
                }
                delete surroundImage;
            }
        }
    }
    pthread_mutex_unlock(&mMutexQueue);
}

void CaptureWorkerV4l2::run()
{
    //clearOverstock();
#if DEBUG_CAPTURE
    double start = clock();
    int size = 0;
    double elapsed = 0;
    double read_time = 0;
    double convert_time = 0;
    if (mLastCallTime > 0.00001f)
    {
        elapsed = (start - mLastCallTime)/CLOCKS_PER_SEC;
    }
    mLastCallTime = start;
#endif

    void* image[VIDEO_CHANNEL_SIZE] = {NULL};
    unsigned char flag = 1;
    long timestamp = Util::get_system_milliseconds();
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

#if DEBUG_CAPTURE
        long read_start = (double)clock();
#endif
        struct v4l2_buffer buf;
        if (-1 != V4l2::readFrame(mVideoFd[i], &buf, mMemType))
        {
            if (buf.index < V4L2_BUF_COUNT)
            {
#if DEBUG_CAPTURE
                read_time = (clock()-read_start)/CLOCKS_PER_SEC;
#endif

#if DEBUG_CAPTURE
                double convert_start = clock();
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
                        << ", read_time:" << read_time
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
            }
        }

        V4l2::v4l2QueueBuf(mVideoFd[i], &buf);
    }

    //integrity
    if (flag == (1 << mVideoChannelNum))
    {
        surround_images_t* surroundImage = new surround_images_t();
        surroundImage->timestamp = timestamp;
        for (unsigned int i = 0; i < mVideoChannelNum; ++i)
        {
            surroundImage->frame[i].timestamp = timestamp;
            surroundImage->frame[i].info.width = mSource[i].width;
            surroundImage->frame[i].info.height = mSource[i].height;
            surroundImage->frame[i].info.pixfmt = mSource[i].pixfmt;
            surroundImage->frame[i].info.size = mSource[i].size;
            surroundImage->frame[i].data = image[i];
        }

        pthread_mutex_lock(&mMutexQueue);
        mSurroundImagesQueue.push(surroundImage);
#if DEBUG_CAPTURE
        size = mSurroundImagesQueue.size();
#endif
        pthread_mutex_unlock(&mMutexQueue);

	    if (mRealFrameCount == 0)
	    {
	        mStartStatTime = clock();
	    }

	    mRealFrameCount++;
        mRealFPS = mRealFrameCount/mStatDuration;
        mStatDuration = (clock() - mStartStatTime)/CLOCKS_PER_SEC;
	    if (mStatDuration > 5*60
            || mStatDuration < 0)
        {
	        mRealFrameCount = 0;
        }
    }
    else
    {
        for (unsigned int i = 0; i < mVideoChannelNum; ++i)
        {
            if (NULL != image[i])
            {
                delete (cv::Mat*)image[i];
            }
        }
    }

#if DEBUG_CAPTURE
    std::cout << "CaptureWorkerV4l2::onCapture"
            << " thread id:" << getTID()
            << ", elapsed to last time:" << elapsed
            << ", capture:" << (clock()-start)/CLOCKS_PER_SEC
            << ", size:" << size
            << ", fps:" << mRealFPS
            << std::endl;
#endif
}
