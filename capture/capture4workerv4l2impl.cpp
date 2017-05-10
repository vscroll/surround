#include "capture4workerv4l2impl.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <sys/ioctl.h>
#if USE_IMX_IPU
#include <linux/ipu.h>
#endif

Capture4WorkerV4l2Impl::Capture4WorkerV4l2Impl() :
    Capture4WorkerBase()
{
    mIPUFd = -1;

    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mVideoFd[i] = -1;

	memset(&mSink[i], 0, sizeof(mSink[i]));
	memset(&mSideSrc[i], 0, sizeof(mSideSrc[i]));
	memset(&mPanoSrc[i], 0, sizeof(mPanoSrc[i]));
        mSinkFrameSize[i] = 0;
        mSideSrcFrameSize[i] = 0;
        mPanoSrcFrameSize[i] = 0;
    }
#if USE_IMX_IPU
    mMemType = V4L2_MEMORY_USERPTR;
#else
    mMemType = V4L2_MEMORY_MMAP;
#endif

    mRealFPS = 0;
    mStartTime = 0.0;
    mStatDuration = 0.0;
    mRealFrameCount = 0;
}

Capture4WorkerV4l2Impl::~Capture4WorkerV4l2Impl()
{

}

void Capture4WorkerV4l2Impl::setCapCapacity(struct cap_sink_t sink[], struct cap_src_t sideSrc[], struct cap_src_t panoSrc[], unsigned int channelNum)
{
    mVideoChannelNum = channelNum <= VIDEO_CHANNEL_SIZE ? channelNum: VIDEO_CHANNEL_SIZE;
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
	memcpy(&mSink[i], sink, sizeof(mSink[i]));
	memcpy(&mSideSrc[i], sideSrc, sizeof(mSideSrc[i]));
	memcpy(&mPanoSrc[i], panoSrc, sizeof(mPanoSrc[i]));
    }	
}

int Capture4WorkerV4l2Impl::openDevice(unsigned int channel[], unsigned int channelNum)
{
#if USE_IMX_IPU
    mIPUFd = open("/dev/mxc_ipu", O_RDWR, 0);
    if (mIPUFd < 0)
    {
        std::cout << "Capture4WorkerV4l2Impl::openDevice"
                  << " open ipu failed"
	          << std::endl;
        return -1;
    }
    std::cout << "Capture4WorkerV4l2Impl::openDevice"
              << " ipu fd:" << mIPUFd
	      << std::endl;

#endif

    unsigned int pixfmt;
    unsigned int width;
    unsigned int height;
    mVideoChannelNum = channelNum <= VIDEO_CHANNEL_SIZE ? channelNum: VIDEO_CHANNEL_SIZE;
    for (unsigned int i = 0; i < mVideoChannelNum; ++i)
    {
        int videoChannel = channel[i];
        char devName[16] = {0};
        sprintf(devName, "/dev/video%d", videoChannel);
        mVideoFd[i] = open(devName, O_RDWR | O_NONBLOCK);
        if (mVideoFd[i] < 0)
        {
            std::cout << "Capture4WorkerV4l2Impl::openDevice"
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
        std::cout << "Capture4WorkerV4l2Impl::openDevice"
                 << " mem type: " << mMemType
                 << " buf count:" << V4L2_BUF_COUNT
                << " in_pixfmt:" << mSink[i].pixfmt
                << " in_width:" << mSink[i].width
                << " in_height:" << mSink[i].height
		<< std::endl;
#endif
	mSideSrc[i].width = mSink[i].width;
	mSideSrc[i].height = mSink[i].height;

	mPanoSrc[i].width = mSink[i].width;
	mPanoSrc[i].height = mSink[i].height;

	if (mSink[i].pixfmt == IN_PIX_FMT_UYVY)
	{
	     mSinkFrameSize[i] = mSink[i].width * mSink[i].height * 2;        
	}

	if (mSideSrc[i].pixfmt == OUT_PIX_FMT_UYVY)
	{
	    mSideSrcFrameSize[i] = mSideSrc[i].width * mSideSrc[i].height * 2;        
	}
	else if (mSideSrc[i].pixfmt == OUT_PIX_FMT_BGR24)
	{
	    mSideSrcFrameSize[i] = mSideSrc[i].width * mSideSrc[i].height * 3;
	}

	if (mPanoSrc[i].pixfmt == OUT_PIX_FMT_UYVY)
	{
	    mPanoSrcFrameSize[i] = mPanoSrc[i].width * mPanoSrc[i].height * 2;        
	}
	else if (mPanoSrc[i].pixfmt == OUT_PIX_FMT_BGR24)
	{
	    mPanoSrcFrameSize[i] = mPanoSrc[i].width * mPanoSrc[i].height * 3;
	}

        for (unsigned int j = 0; j < V4L2_BUF_COUNT; ++j)
        {
            mV4l2Buf[i][j].width = mSink[i].width;
            mV4l2Buf[i][j].height = mSink[i].height;
            mV4l2Buf[i][j].pixfmt = mSink[i].pixfmt;
        }

        if (-1 == V4l2::v4l2ReqBuf(mVideoFd[i], mV4l2Buf[i], V4L2_BUF_COUNT, mMemType, mIPUFd, mSinkFrameSize[i]))
        {
            return -1;
        }

#if USE_IMX_IPU
        if (-1 == IMXIPU::allocIPUBuf(mIPUFd, &(mOutIPUBuf[i]), mPanoSrcFrameSize[i]))
        {
            return -1;
        }
#endif

#if DEBUG_CAPTURE
        std::cout << "Capture4WorkerV4l2Impl::openDevice"
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
        std::cout << "Capture4WorkerV4l2Impl::openDevice"
                 << " startCapture ok"
		 << std::endl;
#endif

    return 0;
}

void Capture4WorkerV4l2Impl::closeDevice()
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

void Capture4WorkerV4l2Impl::getSideResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (channelIndex < VIDEO_CHANNEL_SIZE)
    {
        *width = mSideSrc[channelIndex].width;
        *height = mSideSrc[channelIndex].height;
    }
}

void Capture4WorkerV4l2Impl::getPanoResolution(unsigned int channelIndex, unsigned int* width, unsigned int* height)
{
    if (channelIndex < VIDEO_CHANNEL_SIZE)
    {
        *width = mPanoSrc[channelIndex].width;
        *height = mPanoSrc[channelIndex].height;
    }
}

int Capture4WorkerV4l2Impl::getFPS(unsigned int* fps)
{
#if 0
    unsigned int interval = getInterval();
    if (interval > 0)
    {
        *fps = 1000/interval;
	return 0;
    }
#else
    if (mRealFPS > 0)
    {
        *fps = mRealFPS;
	return 0;
    }

#endif

    return -1;
}

void Capture4WorkerV4l2Impl::run()
{
#if DEBUG_CAPTURE
    double start = (double)clock();
    int size = 0;
    double elapsed = 0;
    double read_time = 0;
    double convert_time = 0;
    if (mLastTimestamp > 0.00001f)
    {
        elapsed = (start - mLastTimestamp)/CLOCKS_PER_SEC;
    }
    mLastTimestamp = start;
#endif

    void* image[VIDEO_CHANNEL_SIZE] = {NULL};
    unsigned char flag = 1;
    double timestamp = (double)clock();
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
                std::cout << "Capture4WorkerV4l2Impl::onCapture"
                         << ", EINTR"
			 << std::endl;
                return;
        }

        if (0 == r) {
            std::cout << "Capture4WorkerV4l2Impl::onCapture"
                     << ", select timeout"
		     << std::endl;
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
                read_time = (clock()-read_start)/CLOCKS_PER_SEC;
#endif

#if DEBUG_CAPTURE
                double convert_start = (double)clock();
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

                task.output.width = mPanoSrc[i].width;
                task.output.height = mPanoSrc[i].height;
                task.output.crop.pos.x = 0;
                task.output.crop.pos.y = 0;
                task.output.crop.w = mPanoSrc[i].width;
                task.output.crop.h = mPanoSrc[i].height;
                //for colour cast
                //task.output.format = V4L2_PIX_FMT_RGB24;
                task.output.format = mPanoSrc[i].pixfmt;

                task.input.paddr = (int)mV4l2Buf[i][buf.index].offset;
                task.output.paddr = (int)mOutIPUBuf[i].offset;
                if (ioctl(mIPUFd, IPU_QUEUE_TASK, &task) < 0) {
                    std::cout << "Capture4WorkerV4l2Impl::onCapture"
                        << ", ipu task failed:" << mIPUFd
			<< std::endl;
                    continue;
                }
                
#else
                unsigned char frame_buffer[mPanoSrcFrameSize[i]];
                Util::uyvy_to_rgb24(mSink[i].width, mSink[i].height, (unsigned char*)(mV4l2Buf[i][buf.index].start), frame_buffer);
#endif

#if DEBUG_CAPTURE
/*
                convert_time = (clock() - convert_start)/CLOCKS_PER_SEC;
                std::cout << "Capture4WorkerV4l2Impl::onCapture"
                         << ", channel:" << i
                         << ", read_time:" << read_time
                         << ", yuv to rgb:" << convert_time
			 << std::endl;
*/
#endif

#if USE_IMX_IPU                
                cv::Mat* matImage = new cv::Mat(mPanoSrc[i].height, mPanoSrc[i].width, CV_8UC3, mOutIPUBuf[i].start);
#else
                cv::Mat* matImage = new cv::Mat(mPanoSrc[i].height, mPanoSrc[i].width, CV_8UC3, frame_buffer);
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
            surroundImage->frame[i].data = image[i];
            surroundImage->frame[i].width = mPanoSrc[i].width;
            surroundImage->frame[i].height = mPanoSrc[i].height;
            surroundImage->frame[i].pixfmt = mPanoSrc[i].pixfmt;
        }

        pthread_mutex_lock(&mMutexQueue);
        mSurroundImageQueue.push(surroundImage);
#if DEBUG_CAPTURE
        size = mSurroundImageQueue.size();
#endif
        pthread_mutex_unlock(&mMutexQueue);

	if (mRealFrameCount == 0)
	{
	    mStartTime = clock();
	}

	mRealFrameCount++;
        mRealFPS = mRealFrameCount/mStatDuration;
        mStatDuration = (clock()-mStartTime)/CLOCKS_PER_SEC;
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
    std::cout << "Capture4WorkerV4l2Impl::onCapture"
	     << " thread id:" << getTID()
             << ", channel:" << mVideoChannelNum
             << ", flag:" << (int)flag
             << ", size:" << size
             << ", elapsed to last time:" << elapsed
             << ", capture:" << (clock()-timestamp)/CLOCKS_PER_SEC
             << ", fps:" << mRealFPS
	     << std::endl;
#endif
}
