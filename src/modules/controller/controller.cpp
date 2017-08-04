#include "controller.h"
#include <iostream>
#include "common.h"
#include "IConfig.h"
#include "configimpl.h"
#include "ICapture.h"
#include "captureimpl.h"
#include "capture1impl.h"
#include "util.h"
#include "v4l2.h"
#include "focussourceshmwriteworker.h"
#include "sourceshmwriteworker.h"
#include "wrap_thread.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

class InputEventWorker : public WrapThread
{
public:
    InputEventWorker(ICapture* capture, unsigned int focusChannelIndex);
    virtual ~InputEventWorker();

public:
    virtual void run();

private:
    ICapture* mCapture;
    unsigned int mFocusChannelIndex;
    int mEventFd;
};

InputEventWorker::InputEventWorker(ICapture* capture, unsigned int focusChannelIndex)
{
    mCapture = capture;
    mFocusChannelIndex = focusChannelIndex;
    mEventFd = -1;
}

InputEventWorker::~InputEventWorker()
{
    if (mEventFd > 0)
    {
        close(mEventFd);
        mEventFd = -1;
    }
}

void InputEventWorker::run()
{
    if (NULL == mCapture)
    {
        return;
    }

    // touch screen event
    if (-1 == mEventFd)
    {
        mEventFd = open("/dev/input/event0", O_RDONLY);
        if (mEventFd < 0)
        {
            return;
        }
    }

    struct input_event event[64] = {0};

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(mEventFd, &fds);

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

     //touch screen to switch focus channel
    int r = select (mEventFd + 1, &fds, NULL, NULL, &tv);
    if (-1 == r) {
        usleep(100);
    }

    if (0 == r) {
        usleep(100);
    }

    int rd = read(mEventFd, event, sizeof(struct input_event) * 64);
    if (rd >= (int) sizeof(struct input_event))
    {
        for (int i = 0; i < rd / sizeof(struct input_event); i++)
        {
            if (event[i].type == EV_KEY
                && event[i].code == BTN_TOUCH
                && event[i].value == 0)
            {
                mFocusChannelIndex++;
                if (mFocusChannelIndex >= VIDEO_CHANNEL_SIZE)
                {
                    mFocusChannelIndex = VIDEO_CHANNEL_FRONT;
                }
                mCapture->setFocusSource(mFocusChannelIndex, NULL);
                break;
            }
        }
    }
}

Controller::Controller()
{
    mConfig = NULL;
    mCapture = NULL;
    mInputEventWorker = NULL;

    mFocusSourceSHMWriteWorker = NULL;
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mSourceSHMWriteWorker[i] = NULL;
    }
}

Controller::~Controller()
{
}

int Controller::initConfigModule()
{
    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }

    char cfgPathName[1024] = {0};
    sprintf(cfgPathName, "%sconfig.ini", procPath);
    mConfig = new ConfigImpl();
    if (mConfig->loadFromFile(cfgPathName) < 0)
    {
        delete mConfig;
        mConfig = NULL;
        return -1;
    }

    return 0;
}

void Controller::uninitConfigModule()
{
    if (NULL != mConfig)
    {
        delete mConfig;
        mConfig = NULL;
    }
}

int Controller::startCaptureModule(bool enableSHM)
{
    if (NULL == mConfig)
    {
        return -1;
    }

    //capture
    int frontChn;
    int rearChn;
    int leftChn;
    int rightChn;
    if (mConfig->getChannelNo(&frontChn, &rearChn, &leftChn, &rightChn) < 0)
    {    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }
        return -1;
    }

    int captureFPS = mConfig->getCaptureFPS();
    if (captureFPS <= 0)
    {
        captureFPS = VIDEO_FPS_15;
    }

	//sink
	int sinkWidth = mConfig->getSinkWidth();
	int sinkHeight = mConfig->getSinkHeight();
	if (sinkWidth < 0
		|| sinkHeight < 0
		|| sinkWidth > CAPTURE_VIDEO_RES_X
		|| sinkHeight > CAPTURE_VIDEO_RES_Y)
	{
	    std::cout << "sink config error"
	            << std::endl;
		return -1;
	}
    char procPath[1024] = {0};
    if (Util::getAbsolutePath(procPath, 1024) < 0)
    {
        return -1;
    }
	//crop
	int cropX[VIDEO_CHANNEL_SIZE];
	int cropY[VIDEO_CHANNEL_SIZE];
	int cropWidth[VIDEO_CHANNEL_SIZE];
	int cropHeight[VIDEO_CHANNEL_SIZE];
	for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
	{
		cropX[i] = mConfig->getSinkCropX(i);
		cropY[i] = mConfig->getSinkCropY(i);
		cropWidth[i] = mConfig->getSinkCropWidth(i);
		cropHeight[i] = mConfig->getSinkCropHeight(i);
		if (cropX[i] < 0
			|| cropY[i] < 0
			|| cropWidth[i] < 0
			|| cropHeight[i] < 0
			|| (cropX[i] + cropWidth[i]) > CAPTURE_VIDEO_RES_X
			|| (cropY[i] + cropHeight[i]) > CAPTURE_VIDEO_RES_Y)
		{
		    std::cout << "sink crop config error"
		            << std::endl;
			return -1;
		}
	}

	int focusSinkCropX = mConfig->getFocusSinkCropX();
	int focusSinkCropY = mConfig->getFocusSinkCropY();
	int focusSinkCropWidth = mConfig->getFocusSinkCropWidth();
	int focusSinkCropHeight = mConfig->getFocusSinkCropHeight();
	if (focusSinkCropX < 0
		|| focusSinkCropY < 0
		|| focusSinkCropX > CAPTURE_VIDEO_RES_X
		|| focusSinkCropY > CAPTURE_VIDEO_RES_Y
		|| focusSinkCropWidth < 0
		|| focusSinkCropHeight < 0
		|| focusSinkCropWidth > CAPTURE_VIDEO_RES_X
		|| focusSinkCropHeight > CAPTURE_VIDEO_RES_Y)
	{
		std::cout << "focus sink crop config error"
				<< std::endl;
		return -1;
	}

	//source
	int srcWidth[VIDEO_CHANNEL_SIZE];
	int srcHeight[VIDEO_CHANNEL_SIZE];
	for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
	{
		srcWidth[i] = mConfig->getSourceWidth(i);
		srcHeight[i] = mConfig->getSourceHeight(i);
		if (srcWidth[i] < 0
			|| srcHeight[i] < 0
			|| srcWidth[i] > CAPTURE_VIDEO_RES_X
			|| srcHeight[i] > CAPTURE_VIDEO_RES_Y)
		{
		    std::cout << "source config error"
		            << std::endl;
			return -1;
		}
	}

	int focusSrcWidth = mConfig->getFocusSourceWidth();
	int focusSrcHeight = mConfig->getFocusSourceHeight();
	if (focusSrcWidth < 0
		|| focusSrcHeight < 0
		|| focusSrcWidth > CAPTURE_VIDEO_RES_X
		|| focusSrcHeight > CAPTURE_VIDEO_RES_Y)
	{
		std::cout << "focus source config error"
				<< std::endl;
		return -1;
	}

#if 0
    mCapture = new CaptureImpl();
#else
    mCapture = new Capture1Impl(VIDEO_CHANNEL_SIZE);
#endif

    unsigned int channel[VIDEO_CHANNEL_SIZE] = {frontChn,rearChn,leftChn,rightChn};
    struct cap_sink_t sink[VIDEO_CHANNEL_SIZE];
    struct cap_src_t source[VIDEO_CHANNEL_SIZE];
    for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        sink[i].pixfmt = V4L2_PIX_FMT_UYVY;
        sink[i].width = CAPTURE_VIDEO_RES_X;
        sink[i].height = CAPTURE_VIDEO_RES_Y;
        sink[i].size = V4l2::getVideoSize(sink[i].pixfmt, sink[i].width, sink[i].height);
        sink[i].crop_x = cropX[i];
        sink[i].crop_y = cropY[i];
        sink[i].crop_w = cropWidth[i];
        sink[i].crop_h = cropHeight[i];

        // source's pixfmt is same as sink
        source[i].pixfmt = sink[i].pixfmt;
        source[i].width = srcWidth[i];
        source[i].height = srcHeight[i];
        source[i].size = V4l2::getVideoSize(source[i].pixfmt, source[i].width, source[i].height);
    }
    if (mCapture->setCapCapacity(sink, source, VIDEO_CHANNEL_SIZE))
    {
        std::cout << "capture_main::setCapCapacity error"
                << std::endl;
        delete mCapture;
        mCapture = NULL;
        return -1;        
    }

    // focus source's pixfmt is same as source
    int focusChannelIndex = VIDEO_CHANNEL_FRONT;
    struct cap_src_t focusSource;
    focusSource.pixfmt = source[0].pixfmt;
    focusSource.width = focusSrcWidth;
    focusSource.height = focusSrcHeight;
    focusSource.size = V4l2::getVideoSize(focusSource.pixfmt, focusSource.width, focusSource.height);
    mCapture->setFocusSource(focusChannelIndex, &focusSource);

    if (mCapture->openDevice(channel, VIDEO_CHANNEL_SIZE) < 0)
    {
        std::cout << "capture_main::openDevice error"
                << std::endl;
        delete mCapture;
        mCapture = NULL;
        return -1;
    }

    mCapture->start(captureFPS);

    if (enableSHM)
    {
        //focus source
        mFocusSourceSHMWriteWorker = new FocusSourceSHMWriteWorker(mCapture);
        mFocusSourceSHMWriteWorker->start(captureFPS);

        //4 source
        for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
        {
            mSourceSHMWriteWorker[i] = new SourceSHMWriteWorker(mCapture, i);
            mSourceSHMWriteWorker[i]->start(captureFPS);
        }
    }

    return 0;
}

void Controller::stopCaptureModule()
{
    for (unsigned int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
    {
        mSourceSHMWriteWorker[i]->stop();
        delete mSourceSHMWriteWorker[i];
        mSourceSHMWriteWorker[i] = NULL;
    }

    if (NULL != mFocusSourceSHMWriteWorker)
    {
        mFocusSourceSHMWriteWorker->stop();
        delete mFocusSourceSHMWriteWorker;
        mFocusSourceSHMWriteWorker = NULL;
    }

    if (NULL != mCapture)
    {
        mCapture->stop();
        delete mCapture;
        mCapture = NULL;
    }    
}

int Controller::startInputEventModule()
{
    if (NULL == mCapture)
    {
        return -1;
    }

    mInputEventWorker = new InputEventWorker(mCapture, VIDEO_CHANNEL_FRONT);
    mInputEventWorker->start(15);

    return 0;
}

void Controller::stopInputEventModule()
{
    if (NULL != mInputEventWorker)
    {
        mInputEventWorker->stop();
        delete mInputEventWorker;
        mInputEventWorker = NULL;
    }

}
