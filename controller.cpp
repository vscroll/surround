#include "controller.h"
#include "ICapture.h"
#include "IStitch.h"
#include "stitchimpl.h"
#include "capture1impl.h"
#include "capture4impl.h"
#include <QDebug>

Controller::Controller() :
    mIsRunning(false),
    mCapture(NULL),
    mStitch(NULL)
{

}

void Controller::init()
{
    if (NULL == mCapture)
    {
        mCapture = new Capture4Impl();
        //mCapture = new Capture1Impl();
        mCapture->openDevice();
    }

    if (NULL == mStitch)
    {
        mStitch = new StitchImpl();
    }
}

void Controller::uninit()
{
    if (NULL != mCapture)
    {
         delete mCapture;
         mCapture = NULL;
    }

    if (NULL != mStitch)
    {
        delete mStitch;
        mStitch = NULL;
    }
}

void Controller::start(VIDEO_FPS captureFps)
{
   if (NULL != mCapture)
   {
       mCapture->start(captureFps);
   }

   if (NULL != mStitch)
   {
       mStitch->start();
   }

   mIsRunning = true;
   QThread::start();
}

void Controller::stop()
{
    mIsRunning = false;

    if (NULL != mCapture)
    {
         mCapture->stop();
         mCapture->closeDevice();
    }

    if (NULL != mStitch)
    {
        mStitch->stop();
    }
}

void Controller::run()
{
    while (true)
    {
        if (!mIsRunning)
        {
            break;
        }

        if (NULL == mCapture
                || NULL == mStitch)
        {
            break;
        }

        surround_image4_t* tmp = mCapture->popOneFrame();
        if (NULL != tmp)
        {
#ifdef DEBUG
            qDebug()<< "Controller::run"
                    << ", elapsed to capture:" << (int)(clock() - tmp->timestamp)/1000;
#endif
            mStitch->append(tmp);
        }

        usleep(1);
    }
}

surround_image1_t* Controller::dequeueFullImage()
{
    surround_image1_t* tmp = NULL;
    if (NULL != mStitch)
    {
        tmp = mStitch->dequeueFullImage();
    }
    return tmp;
}

surround_image1_t* Controller::dequeueSmallImage(VIDEO_CHANNEL channel)
{
    surround_image1_t* tmp = NULL;
    if (NULL != mStitch)
    {
        tmp = mStitch->dequeueSmallImage(channel);
    }
    return tmp;
}
