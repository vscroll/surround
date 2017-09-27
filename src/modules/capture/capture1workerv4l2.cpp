#include "capture1workerv4l2.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "util.h"
#include <sys/ioctl.h>
#include <ostream>
#include <linux/ipu.h>


Capture1WorkerV4l2::Capture1WorkerV4l2() :
    Capture1WorkerBase()
{
    mIPUFd = -1;

    mMemType = V4L2_MEMORY_USERPTR;

    pthread_mutex_init(&mMutexV4l2, NULL);
    pthread_mutex_init(&mMutexIpu, NULL);
}

Capture1WorkerV4l2::~Capture1WorkerV4l2()
{

}

int Capture1WorkerV4l2::openDevice(unsigned int channel)
{
    mVideoChannel = channel;

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

    //V4l2::setVideoFmt(mVideoFd, mSink.pixfmt, mSink.width, mSink.height);
    //V4l2::getVideoFmt(mVideoFd, &mSink.pixfmt, &mSink.width, &mSink.height);
    //V4l2::setFps(mVideoFd, 15);
    //V4l2::getFps(mVideoFd);

    if (V4l2::setVideoFmt(mVideoFd, mSink.pixfmt, mSink.width, mSink.height) < 0)
    {
        return -1;
    }
#if DEBUG_CAPTURE
    std::cout << "Capture1WorkerV4l2::openDevice:" << devName
            << " mem type: " << mMemType
            << " buf count:" << V4L2_BUF_COUNT
            << " in_pixfmt:" << mSink.pixfmt
            << " in_width:" << mSink.width
            << " in_height:" << mSink.height
            << " in_size:" << mSink.size
            << std::endl;
#endif

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

    if (-1 == IMXIPU::allocIPUBuf(mIPUFd, &mOutIPUBuf, mSource.size))
    {
        return -1;
    }

#if DEBUG_CAPTURE
    std::cout << "Capture1WorkerV4l2::openDevice:" << devName
            << " initV4l2Buf ok"
            << std::endl;
#endif

    if (-1 == V4l2::startCapture(mVideoFd, mV4l2Buf, V4L2_BUF_COUNT, mMemType))
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
			//enhance, convert, crop, resize
            if(isNeedConvert(&mSink, &mSource))
            {            
                // IPU can improve image quality, even though the source is same as the sink
                //YUYV/YVYU/UYVY/VYUY:  in planes[0], buffer address is with 16bytes alignment.
                //width*height*2 % 16 = 0
                struct ipu_task task;
                memset(&task, 0, sizeof(struct ipu_task));
                task.input.width  = mSink.width;
                task.input.height = mSink.height;
                task.input.crop.pos.x = mSink.crop_x;
                task.input.crop.pos.y = mSink.crop_y;
                task.input.crop.w = mSink.crop_w;
                task.input.crop.h = mSink.crop_h;
                task.input.format = V4l2::getIPUPixfmt(mSink.pixfmt);
                task.input.deinterlace.enable = 1;
                task.input.deinterlace.motion = 2;

                task.output.width = mSource.width;
                task.output.height = mSource.height;
                task.output.crop.pos.x = 0;
                task.output.crop.pos.y = 0;
                task.output.crop.w = mSource.width;
                task.output.crop.h = mSource.height;
                task.output.format = V4l2::getIPUPixfmt(mSource.pixfmt);

                task.input.paddr = (int)mV4l2Buf[buf.index].offset;
                task.output.paddr = (int)mOutIPUBuf.offset;
                if (ioctl(mIPUFd, IPU_QUEUE_TASK, &task) < 0) {
                    std::cout << "CaptureWorkerV4l2::onCapture"
                            << ", ipu task failed:" << mIPUFd
                            << std::endl;
                    return;
                }
			}

			//for focus channel
            if (!isNeedConvert(&mSink, &mFocusSource))
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
				surround_image->pAddr = mV4l2Buf[buf.index].offset;
                //surround_image->data = mOutIPUBuf.start;

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
                    mCaptureFrame4FocusSource.pAddr = surround_image->pAddr;
                }
            }


			//for panorama
            surround_image_t* surround_image = new surround_image_t();
            surround_image->timestamp = Util::get_system_milliseconds();
            surround_image->info.pixfmt = mSource.pixfmt;
            surround_image->info.width = mSource.width;
            surround_image->info.height = mSource.height;
            surround_image->info.size = mSource.size;
            //surround_image->data = new unsigned char[mFocusSource.size];
            //memcpy((unsigned char*)surround_image->data, (unsigned char*)mV4l2Buf[i][buf.index].start, mFocusSource.size);
            if(isNeedConvert(&mSink, &mSource))
            {
                surround_image->data = mOutIPUBuf.start;
                surround_image->pAddr = mOutIPUBuf.offset;
            }
            else
            {
                surround_image->data = mV4l2Buf[buf.index].start;
                surround_image->pAddr = mV4l2Buf[buf.index].offset;
            }        

            pthread_mutex_lock(&mMutexQueue);
            mSurroundImageQueue.push(surround_image);
#if DEBUG_CAPTURE
            size = mSurroundImageQueue.size();
#endif
            pthread_mutex_unlock(&mMutexQueue);
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
