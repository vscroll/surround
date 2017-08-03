#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"

class IConfig;
class ICapture;
class FocusSourceSHMWriteWorker;
class SourceSHMWriteWorker;
class Controller
{
public:
    Controller();
    virtual ~Controller();

    int initConfigModule();
    void uninitConfigModule();

    int startCaptureModule(bool enableSHM);
    void stopCaptureModule();
    void updateFocusChannel(int channelIndex);
protected:
    IConfig* mConfig;
    ICapture* mCapture;
    FocusSourceSHMWriteWorker* mFocusSourceSHMWriteWorker;
    SourceSHMWriteWorker* mSourceSHMWriteWorker[VIDEO_CHANNEL_SIZE];
};

#endif // CONTROLLER_H
