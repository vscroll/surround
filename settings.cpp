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
    mEnableOpenCL = 0;
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

    int value = settings.value("INPUT/FrontCHN").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_FRONT] = value;
    }

    value = settings.value("INPUT/RearCHN").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_REAR] = value;
    }

    value = settings.value("INPUT/LeftCHN").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_LEFT] = value;
    }

    value = settings.value("INPUT/RightCHN").toInt();
    if (value >= 0)
    {
        mVideoChanel[VIDEO_CHANNEL_RIGHT] = value;
    }

    value = settings.value("INPUT/CaptureFPS").toInt();
    if (value >= 0)
    {
        mFps = value;
    }

    value = settings.value("OUTPUT/Pano2DWidth").toInt();
    if (value >= 0)
    {
        mPano2DWidth = value;
    }

    value = settings.value("OUTPUT/Pano2DHeight").toInt();
    if (value >= 0)
    {
        mPano2DHeight = value;
    }

    value = settings.value("ACCEL/EnableOpenCL").toInt();
    if (value >= 0)
    {
        mEnableOpenCL = value;
    }


    qDebug() << "FrontCHN:" << mVideoChanel[VIDEO_CHANNEL_FRONT]
        << " RearCHN:" << mVideoChanel[VIDEO_CHANNEL_REAR]
        << " LeftCHN:" << mVideoChanel[VIDEO_CHANNEL_LEFT]
        << " RightCHN:" << mVideoChanel[VIDEO_CHANNEL_RIGHT]
        << " CaptureFPS:" << mFps;

    qDebug() << "Pano2D width:" << mPano2DWidth
        << " Pano2D height:" << mPano2DHeight;

    qDebug() << "EnableOpenCL:" << mEnableOpenCL;
}

QString Settings::getApplicationPath()
{
    return QCoreApplication::applicationDirPath();
}
