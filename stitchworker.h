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

    surround_image1_t* dequeueFullImage();
    surround_image1_t* dequeueSmallImage(VIDEO_CHANNEL channel);

protected:
    void run();

signals:

public slots:

private:
    static const int FULL_WIDTH = 424;
    static const int FULL_HEIGHT = 600;

    bool mIsRunning;
    QQueue<surround_image1_t*> mOutputFullImageQueue;
    QQueue<surround_image1_t*> mOutputSmallImageQueue;

    QMutex mOutputFullImageMutex;
    QMutex mOutputSmallImageMutex;

    VIDEO_CHANNEL mVideoChannel;

    ICapture *mCapture;
    int mFreq;

    cv::Mat mStitchMap;
    cv::Mat mMask;
};

#endif // STITCHWORKER_H
