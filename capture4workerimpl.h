#ifndef CAPTUREWORKERIMPL_H
#define CAPTUREWORKERIMPL_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include "common.h"
#include "capture4workerbase.h"

class CvCapture;
class Capture4WorkerImpl : public Capture4WorkerBase
{
    Q_OBJECT
public:
    explicit Capture4WorkerImpl(QObject *parent = 0, int videoChannelNum = 4);

public:
    virtual void openDevice();
    virtual void closeDevice();
signals:

public slots:
    virtual void onCapture();
private:
    CvCapture *mCaptureArray[VIDEO_CHANNEL_SIZE];
};

#endif // CAPTUREWORKERIMPL_H
