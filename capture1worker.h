#ifndef CAPTURE1WORKER_H
#define CAPTURE1WORKER_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include "common.h"

class CvCapture;
class Capture1Worker : public QObject
{
    Q_OBJECT
public:
    explicit Capture1Worker(QObject *parent = 0, int videoChannel = VIDEO_CHANNEL_FRONT);

    void openDevice();
    void closeDevice();
    surround_image1_t* popOneFrame();
    int getFrameCount();
signals:

public slots:
    void onCapture();
private:
    int mVideoChannel;
    QQueue<surround_image1_t*> mSurroundImageQueue;
    QMutex mMutex;

    CvCapture *mCapture;

    int mDropFrameCount;
    double mLastTimestamp;
};

#endif // CAPTURE1WORKER_H
