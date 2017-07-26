#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"

class IConfig;
class ICapture;
class IPanoImage;
class FocusSourceSHMWriteWorker;
class SourceSHMWriteWorker;
class PanoSourceSHMWriteWorker;
class Controller
{
public:
    Controller();
    virtual ~Controller();

    int initConfigModule();
    void uninitConfigModule();

    int startCaptureModule();
    void stopCaptureModule();
    void updateFocusChannel(int channelIndex);

    int startPanoImageModule();   
    void stopPanoImageModule();

private:
    IConfig* mConfig;
    ICapture* mCapture;
    IPanoImage* mPanoImage;
    FocusSourceSHMWriteWorker* mFocusSourceSHMWriteWorker;
    SourceSHMWriteWorker* mSourceSHMWriteWorker[VIDEO_CHANNEL_SIZE];
    PanoSourceSHMWriteWorker* mPanoSourceSHMWriteWorker;
};

#endif // CONTROLLER_H
