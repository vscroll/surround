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
    int mFullWidth;
    int mFullHeight;

    QQueue<surround_image_t*> mOutputFullImageQueue;
    QQueue<surround_image_t*> mOutputSmallImageQueue;

    QMutex mOutputFullImageMutex;
    QMutex mOutputSmallImageMutex;

    VIDEO_CHANNEL mVideoChannel;

    ICapture *mCapture;

    cv::Mat mStitchMap;
    cv::Mat mMask;

    double mLastTimestamp;
};

#endif // STITCHWORKER_H
