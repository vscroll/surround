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
#include "imxipu.h"
#include "wrap_thread.h"
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <opencv/cv.h>

class InputEventWorker : public WrapThread
{
public:
    InputEventWorker(Controller* controller);
    virtual ~InputEventWorker();

public:
    virtual void run();

private:
    Controller* mController;
    unsigned int mFocusChannelIndex;
    int mEventFd;
};

InputEventWorker::InputEventWorker(Controller* controller)
{
    mController = controller;
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
    if (NULL == mController)
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

    int x = -1;
    int y = -1;

    int rd = read(mEventFd, event, sizeof(struct input_event) * 64);
    if (rd >= (int) sizeof(struct input_event))
    {
        for (int i = 0; i < rd / sizeof(struct input_event); i++)
        {
            if (event[i].type == EV_ABS
                && event[i].code == ABS_MT_POSITION_X)
            {
                x = event[i].value;
            }

            if (event[i].type == EV_ABS
                && event[i].code == ABS_MT_POSITION_Y)
            {
                y = event[i].value;
            }

            if (event[i].type == EV_KEY
                && event[i].code == BTN_TOUCH)
            {
                std::cout << "(x,y):" << x << "," << y << std::endl;
                if (x >= 424)
                {
            	    std::cout << "Controller::EVENT_UPDATE_FOCUS_CHANNEL" << std::endl;
                    mController->procEvent(Controller::EVENT_UPDATE_FOCUS_CHANNEL);
                }
                else if (x >= 0)
                {
                    if (y >= 300)
                    {
                        std::cout << "Controller::EVENT_UPDATE_PANORAMA_VIEW" << std::endl;
                        mController->procEvent(Controller::EVENT_UPDATE_PANORAMA_VIEW);
                    }
                    else
                    {
                        std::cout << "Controller::EVENT_CAPTURE_IMAGE" << std::endl;
                        mController->procEvent(Controller::EVENT_CAPTURE_IMAGE);
                    }
                }
                else
                {
                }
            }
        }
    }
}

Controller::Controller()
{
    mConfig = NULL;
    mCapture = NULL;
    mInputEventWorker = NULL;
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
    {
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

    int sinkPixfmt = mConfig->getSinkPixfmt();

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

    int srcPixfmt = mConfig->getSourcePixfmt();

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
        sink[i].pixfmt = V4l2::getPixfmt(sinkPixfmt);
        sink[i].width = CAPTURE_VIDEO_RES_X;
        sink[i].height = CAPTURE_VIDEO_RES_Y;
        sink[i].size = V4l2::getVideoSize(sink[i].pixfmt, sink[i].width, sink[i].height);
        sink[i].crop_x = cropX[i];
        sink[i].crop_y = cropY[i];
        sink[i].crop_w = cropWidth[i];
        sink[i].crop_h = cropHeight[i];

        source[i].pixfmt = V4l2::getPixfmt(srcPixfmt);
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

    // focus source's pixfmt is same as sink
    int focusChannelIndex = VIDEO_CHANNEL_FRONT;
    struct cap_src_t focusSource;
    focusSource.pixfmt = V4l2::getPixfmt(sinkPixfmt);
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

    return 0;
}

void Controller::stopCaptureModule()
{
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

    mInputEventWorker = new InputEventWorker(this);
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

void Controller::procEvent(int event)
{
    if (NULL == mCapture)
    {
        return;
    }

    switch (event)
    {
        case EVENT_CAPTURE_IMAGE:
        {
            surround_images_t* surroundImage = mCapture->popOneFrame();
            if (NULL != surroundImage)
            {
                std::cout << "Controller::procEvent EVENT_CAPTURE_IMAGE"
                        << std::endl;

                for (int i = 0; i < VIDEO_CHANNEL_SIZE; ++i)
                {
                    unsigned char* buffer = (unsigned char*)surroundImage->frame[i].data;
                    int width = surroundImage->frame[i].info.width;
                    int height = surroundImage->frame[i].info.height;
                    int pixfmt = surroundImage->frame[i].info.pixfmt;
                    int size = surroundImage->frame[i].info.size;
                    if (pixfmt == V4L2_PIX_FMT_RGB32)
                    {
                        cv::Mat image = cv::Mat(height, width, CV_8UC4, buffer);
                        IplImage iplImage = IplImage(image);
                        Util::write2File(i, &iplImage);
                    }
                }
            }
            break;
        }          
        default:
            break;
    }
}
