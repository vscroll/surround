#ifndef CAPTURE4WORKERBASE_H
#define CAPTURE4WORKERBASE_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include "common.h"

class Capture4WorkerBase : public QObject
{
    Q_OBJECT
public:
    explicit Capture4WorkerBase(QObject *parent = 0, int videoChannelNum = 4);

    virtual int openDevice();
    virtual void closeDevice();
    virtual surround_image4_t* popOneFrame();
    virtual int getFrameCount();
    virtual int getVideoChannelNum() { return mVideoChannelNum; }
signals:

public slots:
    virtual void onCapture();

protected:
    int mVideoChannelNum;
    QQueue<surround_image4_t*> mSurroundImageQueue;
    QMutex mMutexQueue;

    int mDropFrameCount;
    double mLastTimestamp;
};

#endif // CAPTURE4WORKERBASE_H
