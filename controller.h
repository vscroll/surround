#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include "common.h"

class ICapture;
class IStitch;
class Controller : public QThread
{
    Q_OBJECT
public:
    explicit Controller();

    void init();
    void uninit();
    void start(int fps);
    void stop();
    surround_image_t* dequeueFullImage();
    surround_image_t* dequeueSmallImage(VIDEO_CHANNEL channel);

protected:
    void run();

signals:

public slots:

private:
    bool mIsRunning;
    ICapture* mCapture;
    IStitch* mStitch;
};

#endif // CONTROLLER_H
