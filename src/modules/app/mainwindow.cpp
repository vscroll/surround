#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QPainter>
#include <QDir>
#include <QDebug>
#include "settings.h"
#include "util.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoBackground);
    ui->label_video_small->setAttribute(Qt::WA_NoBackground);
    ui->label_video_full->setAttribute(Qt::WA_NoBackground);

    //mSettings = Settings::getInstant();
    //QString path = mSettings->getApplicationPath() + "/config.ini";
    //qDebug() << "path " << path;
    //mSettings->loadSettings(path);

    //mCurVideoChannel = VIDEO_CHANNEL_FRONT;
    connect(&mVideoUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdate()));
    //connect(&mVideoUpdateSmallTimer, SIGNAL(timeout()), this, SLOT(onUpdateSmallImage()));
    //connect(&mVideoUpdateFullTimer, SIGNAL(timeout()), this, SLOT(onUpdateFullImage()));
    connect(ui->pb_front, SIGNAL(clicked()), this, SLOT(onClickFront()));
    connect(ui->pb_rear, SIGNAL(clicked()), this, SLOT(onClickRear()));
    connect(ui->pb_left, SIGNAL(clicked()), this, SLOT(onClickLeft()));
    connect(ui->pb_right, SIGNAL(clicked()), this, SLOT(onClickRight()));

    //mCaptureFPS = mSettings->mCaptureFps;
    //mUpdateFPS = mSettings->mUpdateFps;
    mUpdateFPS = 15;

    mLastUpdateSmall = 0.0;
    mLastUpdateFull = 0.0;

    start();

    mRealFrameCount = 0;
}

MainWindow::~MainWindow()
{    
    stop();

    delete ui;
}

QImage MainWindow::cvMat2QImage(const cv::Mat& mat)
{    
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

void MainWindow::start()
{
    mVideoUpdateTimer.start(1000/mUpdateFPS);
    //mVideoUpdateSmallTimer.start(1000/mUpdateFPS);
    //mVideoUpdateFullTimer.start(1000/mUpdateFPS);
    mSideSHM.create((key_t)SHM_SIDE_ID, SHM_SIDE_SIZE);
    mPanoSHM.create((key_t)SHM_PANO2D_ID, SHM_PANO2D_SIZE);
}

void MainWindow::stop()
{

}

void MainWindow::onControllerQuit()
{

}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    //updateSmallImage();
    updateFullImage();
}

void MainWindow::onUpdate()
{
    update();
}

void MainWindow::updateFullImage()
{
#if DEBUG_UPDATE
    double start = clock();
    double showElapsed = 0;
    if (mLastUpdateFull > 0.00001f)
    {
        showElapsed = (start - mLastUpdateFull)/CLOCKS_PER_SEC;
    }
    mLastUpdateFull = start;
#endif
    unsigned char imageBuf[SHM_PANO2D_SIZE] = {};
    if (mPanoSHM.readImage(imageBuf, sizeof(imageBuf)) < 0)
    {
        return;
    }
    image_shm_header_t* header = (image_shm_header_t*)imageBuf;

    long elapsed = (Util::get_system_milliseconds() - header->timestamp);
#if DEBUG_UPDATE
    long timestamp = header->timestamp;
    double start1 = clock();
#endif

    //if ((int)(elapsed*1000) < 1000/mFPS)
    {
        cv::Mat side = cv::Mat(header->height, header->width, CV_8UC3, imageBuf+sizeof(image_shm_header_t));
        QImage image = cvMat2QImage(side);
        ui->label_video_full->setPixmap(QPixmap::fromImage(image));
        //QPainter painter(this);
        //painter.drawImage(QPoint(20,20), image);
 #if DEBUG_UPDATE
	    if (mRealFrameCount == 0)
	    {
	        mStartTime = clock();
	    }

	    mRealFrameCount++;
        mStatDuration = (clock()-mStartTime)/CLOCKS_PER_SEC;
	    if (mStatDuration > 5*60
		    || mStatDuration < 0)
        {
	        mRealFrameCount = 0;
        }
#endif        
    }

#if DEBUG_UPDATE
    double end1 = clock();

    qDebug() << "MainWindow::onUpdateFullImage"
             << ", update fps:" << mUpdateFPS
             << ", real update fps:" << mRealFrameCount/mStatDuration
             << ", elapsed to last update:" << showElapsed
             << ", timestamp:" << timestamp
             << ", elapsed to capture:" << elapsed
             << ", show:" << (end1-start1)/CLOCKS_PER_SEC;

#endif
}

void MainWindow::updateSmallImage()
{
#if DEBUG_UPDATE
    double start = clock();
    double showElapsed = 0;
    if (mLastUpdateSmall > 0.00001f)
    {
        showElapsed = (start - mLastUpdateSmall)/CLOCKS_PER_SEC;
    }
    mLastUpdateSmall = start;
#endif
    unsigned char imageBuf[SHM_SIDE_SIZE] = {};
    if (mSideSHM.readImage(imageBuf, sizeof(imageBuf)) < 0)
    {
        return;
    }
    image_shm_header_t* header = (image_shm_header_t*)imageBuf;

    long elapsed = Util::get_system_milliseconds() - header->timestamp;
#if DEBUG_UPDATE
    long timestamp = header->timestamp;
    double start1 = clock();
#endif
    //if ((int)(elapsed*1000) < 1000/mFPS)
    {
        cv::Mat side = cv::Mat(header->height, header->width, CV_8UC3, imageBuf+sizeof(image_shm_header_t));
        QImage image = cvMat2QImage(side);
        ui->label_video_small->setPixmap(QPixmap::fromImage(image));
    }

#if DEBUG_UPDATE
    double end1 = clock();
    qDebug() << "MainWindow::onUpdateSmallImage"
             << ", update fps:" << mUpdateFPS
             << ", elapsed to last update:" << showElapsed
             << ", timestamp:" << timestamp
             << ", elapsed to capture:" << elapsed
             << ", show:" << (end1-start1)/CLOCKS_PER_SEC;
#endif
}

void MainWindow::onUpdateFullImage()
{
    updateFullImage();
}

void MainWindow::onUpdateSmallImage()
{
    updateSmallImage();
}

void MainWindow::onClickFront()
{
    mCurVideoChannel = VIDEO_CHANNEL_FRONT;
}

void MainWindow::onClickRear()
{
    mCurVideoChannel = VIDEO_CHANNEL_REAR;
}

void MainWindow::onClickLeft()
{
    mCurVideoChannel = VIDEO_CHANNEL_LEFT;
}

void MainWindow::onClickRight()
{
    mCurVideoChannel = VIDEO_CHANNEL_RIGHT;
}
