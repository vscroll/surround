#ifndef CAPTUREWORKER_H
#define CAPTUREWORKER_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include "common.h"

class CvCapture;
class Capture4Worker : public QObject
{
    Q_OBJECT
public:
    explicit Capture4Worker(QObject *parent = 0, int videoChannelNum = 4);

public:

    void openDevice();
    void closeDevice();
    surround_image4_t* popOneFrame();
    int getVideoChannelNum() { return mVideoChannelNum; }

signals:

public slots:
    void onCapture();
private:
    int mVideoChannelNum;
    QQueue<surround_image4_t*> mSurroundImageQueue;
    QMutex mMutex;

    CvCapture *mCaptureArray[VIDEO_CHANNEL_SIZE];

    int mDropFrameCount;
    double mLastTimestamp;
};

#endif // CAPTUREWORKER_H
