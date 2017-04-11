#include "settings.h"
#include <QSettings>
#include <QDebug>
#include <QApplication>

Settings* Settings::mInstant = NULL;

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    memset(mVideoChanel, 0, sizeof(mVideoChanel));
    mFullWidth = 0;
    mFullHeight = 0;
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

    value = settings.value("SIZE/fullwidth").toInt();
    if (value >= 0)
    {
        mFullWidth = value;
    }

    value = settings.value("SIZE/fullheight").toInt();
    if (value >= 0)
    {
        mFullHeight = value;
    }

    qDebug() << "front:" << mVideoChanel[VIDEO_CHANNEL_FRONT]
        << " rear:" << mVideoChanel[VIDEO_CHANNEL_REAR]
        << " left:" << mVideoChanel[VIDEO_CHANNEL_LEFT]
        << " right:" << mVideoChanel[VIDEO_CHANNEL_RIGHT];

    qDebug() << "full width:" << mFullWidth
        << " full height:" << mFullHeight;
}

QString Settings::getApplicationPath()
{
    return QCoreApplication::applicationDirPath();
}
