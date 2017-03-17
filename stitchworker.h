#ifndef STITCHWORKER_H
#define STITCHWORKER_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QThread>

#include "common.h"

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
    bool mIsRunning;
    QQueue<surround_image1_t*> mOutputFullImageQueue;
    QQueue<surround_image1_t*> mOutputSmallImageQueue;

    QMutex mOutputFullImageMutex;
    QMutex mOutputSmallImageMutex;

    VIDEO_CHANNEL mVideoChannel;

    ICapture *mCapture;
    int mFreq;
};

#endif // STITCHWORKER_H
