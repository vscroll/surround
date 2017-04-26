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
    int mVideoChanel[VIDEO_CHANNEL_SIZE];
    int mFps;

    int mPano2DWidth;
    int mPano2DHeight;

    int mEnableOpenCL;
signals:

public slots:

private:
    static Settings* mInstant;
};

#endif // SETTINGS_H
