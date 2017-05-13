#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include "common.h"

class Settings : public QObject
{
    Q_OBJECT
private:
    explicit Settings(QObject *parent = 0);

public:
    static Settings* getInstant();

    void loadSettings(QString path);

    QString getApplicationPath();

public:
    unsigned int mVideoChanel[VIDEO_CHANNEL_SIZE];
    unsigned int mCaptureFps;
    unsigned int mUpdateFps;

    unsigned int mPano2DWidth;
    unsigned int mPano2DHeight;

    unsigned int mEnableOpenCL;
signals:

public slots:

private:
    static Settings* mInstant;
};

#endif // SETTINGS_H
