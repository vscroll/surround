#ifndef STITCHIMPL_H
#define STITCHIMPL_H

#include <QObject>
#include <QThread>
#include "IStitch.h"

class StitchWorker;
class StitchImpl : public QObject, public IStitch
{
    Q_OBJECT
public:
    explicit StitchImpl(QObject *parent = 0);
    virtual ~StitchImpl();

    virtual void start(ICapture* capture);
    virtual void stop();
    virtual surround_image_t* dequeueFullImage();
    virtual surround_image_t* dequeueSmallImage(VIDEO_CHANNEL channel);
signals:

public slots:

private:
    StitchWorker *mWorker;
};

#endif // STITCHIMPL_H
