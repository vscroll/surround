#include "settings.h"
#include <QSettings>
#include <QDebug>
#include <QApplication>

Settings* Settings::mInstant = NULL;

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    memset(mVideoChanel, 0, sizeof(mVideoChanel));
    mPano2DWidth = 0;
    mPano2DHeight = 0;

    mFps = 10;
}

Settings* Settings::getInstant()
{
    if (NULL == mInstant)
    {
        mInstant = new Settings();
    }

    return mInstant;
}

void Settings::loadSettings(QString path)
{
    QSettings settings(path, QSettings::IniFormat);

    int value = settings.value("CHN/front").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_FRONT] = value;
    }

    value = settings.value("CHN/rear").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_REAR] = value;
    }

    value = settings.value("CHN/left").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_LEFT] = value;
    }

    value = settings.value("CHN/right").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_RIGHT] = value;
    }

    value = settings.value("SIZE/Pano2DWidth").toInt();
    if (value >= 0)
    {
        mPano2DWidth = value;
    }

    value = settings.value("SIZE/Pano2DHeight").toInt();
    if (value >= 0)
    {
        mPano2DHeight = value;
    }

    value = settings.value("FPS/capture").toInt();
    if (value >= 0)
    {
        mFps = value;
    }

    qDebug() << "front:" << mVideoChanel[VIDEO_CHANNEL_FRONT]
        << " rear:" << mVideoChanel[VIDEO_CHANNEL_REAR]
        << " left:" << mVideoChanel[VIDEO_CHANNEL_LEFT]
        << " right:" << mVideoChanel[VIDEO_CHANNEL_RIGHT];

    qDebug() << "Pano2D width:" << mPano2DWidth
        << " Pano2D height:" << mPano2DHeight
        << " capture fps:" << mFps;
}

QString Settings::getApplicationPath()
{
    return QCoreApplication::applicationDirPath();
}
