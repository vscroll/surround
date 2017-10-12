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
    static const int EVENT_UPDATE_FOCUS_CHANNEL = 0;
    static const int EVENT_UPDATE_PANORAMA_VIEW = 1;
    static const int EVENT_CAPTURE_IMAGE = 2;

public:
    Controller();
    virtual ~Controller();

    virtual void procEvent(int event);

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
