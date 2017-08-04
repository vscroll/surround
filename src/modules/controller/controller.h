#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"

class IConfig;
class ICapture;
class FocusSourceSHMWriteWorker;
class SourceSHMWriteWorker;
class InputEventWorker;
class Controller
{
public:
    Controller();
    virtual ~Controller();

    int initConfigModule();
    void uninitConfigModule();

    int startCaptureModule(bool enableSHM);
    void stopCaptureModule();

    int startInputEventModule();
    void stopInputEventModule();
protected:
    IConfig* mConfig;
    ICapture* mCapture;
    InputEventWorker* mInputEventWorker;
    FocusSourceSHMWriteWorker* mFocusSourceSHMWriteWorker;
    SourceSHMWriteWorker* mSourceSHMWriteWorker[VIDEO_CHANNEL_SIZE];
};

#endif // CONTROLLER_H
