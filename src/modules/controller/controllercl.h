#ifndef CONTROLLERCL_H
#define CONTROLLERCL_H

#include "controller.h"

class IPanoImage;
class PanoSourceSHMWriteWorker;
class ControllerCL : public Controller
{
public:
    ControllerCL();
    virtual ~ControllerCL();

    int startPanoImageModule();   
    void stopPanoImageModule();

private:
    IPanoImage* mPanoImage;
    PanoSourceSHMWriteWorker* mPanoSourceSHMWriteWorker;
};

#endif // CONTROLLERCL_H
