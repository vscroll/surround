#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QThread>

#include "common.h"
#include <opencv/cv.h>

class ICapture;
class StitchWorker : public QThread
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
    void run();

signals:

public slots:

private:
    int mFullWidth;
    int mFullHeight;

    bool mIsRunning;
    QQueue<surround_image_t*> mOutputFullImageQueue;
    QQueue<surround_image_t*> mOutputSmallImageQueue;

    QMutex mOutputFullImageMutex;
    QMutex mOutputSmallImageMutex;

    VIDEO_CHANNEL mVideoChannel;

    ICapture *mCapture;
    int mFreq;

    cv::Mat mStitchMap;
    cv::Mat mMask;
};

#endif // STITCHWORKER_H
