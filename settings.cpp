#include "settings.h"
#include <QSettings>
#include <QDebug>

Settings* Settings::mInstant = NULL;

Settings::Settings(QObject *parent) :
    QObject(parent),
    mVideoChanel({0,0,0,0})
{
    memset(mVideoChanel, 0, sizeof(mVideoChanel));
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

    qDebug() << "front" << mVideoChanel[VIDEO_CHANNEL_FRONT];
    qDebug() << "rear" << mVideoChanel[VIDEO_CHANNEL_REAR];
    qDebug() << "left" << mVideoChanel[VIDEO_CHANNEL_LEFT];
    qDebug() << "right" << mVideoChanel[VIDEO_CHANNEL_RIGHT];
}
