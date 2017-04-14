#ifndef CAPTURE1WORKERBASE_H
#define CAPTURE1WORKERBASE_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include <opencv/cv.h>
#include "common.h"

class Capture1WorkerBase : public QObject
{
    Q_OBJECT
public:
    explicit Capture1WorkerBase(QObject *parent = 0, int videoChannel = 0);

    virtual int openDevice();
    virtual void closeDevice();
    virtual surround_image_t* popOneFrame();
    virtual int getFrameCount();

signals:

public slots:
    virtual void onCapture();

protected:
    void write2File(IplImage* image);

protected:
    int mVideoChannel;
    QQueue<surround_image_t*> mSurroundImageQueue;
    QMutex mMutexQueue;

    int mIPUFd;

    double mLastTimestamp;
    QMutex mMutexFile;
};

#endif // CAPTURE1WORKERBASE_H
