#ifndef CAPTURE1WORKERIMPL_H
#define CAPTURE1WORKERIMPL_H

#include <QObject>
#include <QQueue>
#include <QMutex>

#include "common.h"
#include "capture1workerbase.h"

class CvCapture;
class Capture1WorkerImpl : public Capture1WorkerBase
{
    Q_OBJECT
public:
    explicit Capture1WorkerImpl(QObject *parent = 0, int videoChannel = 0);

    virtual void openDevice();
    virtual void closeDevice();
signals:

public slots:
    virtual void onCapture();
private:
    CvCapture *mCapture;
    QMutex mMutexCapture;

    int mDropFrameCount;
    QMutex mMutexDrop;
};

#endif // CAPTURE1WORKERIMPL_H
