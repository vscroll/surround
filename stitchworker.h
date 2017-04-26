#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include "common.h"
#include <opencv/cv.h>

class ICapture;
class StitchWorker : public QObject
{
    Q_OBJECT
public:
    explicit StitchWorker();

    void start(ICapture* capture);
    void stop();

    surround_image_t* dequeueFullImage();
    surround_image_t* dequeueSmallImage(VIDEO_CHANNEL channel);

private:

protected:
    //void run();

signals:

public slots:
    virtual void onStitch();

private:
    int mPano2DWidth;
    int mPano2DHeight;

    QQueue<surround_image_t*> mOutputFullImageQueue;
    QQueue<surround_image_t*> mOutputSmallImageQueue;

    QMutex mOutputFullImageMutex;
    QMutex mOutputSmallImageMutex;

    VIDEO_CHANNEL mVideoChannel;

    ICapture *mCapture;

    cv::Mat mStitchMap;
    cv::Mat mStitchMapX;
    cv::Mat mStitchMapY;
    cv::Mat mMask;

    double mLastTimestamp;

    bool mEnableOpenCL;
};

#endif // STITCHWORKER_H
