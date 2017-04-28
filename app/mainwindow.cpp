#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QPainter>
#include <QDir>
#include <QDebug>
#include "settings.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NoBackground);
    ui->label_video_small->setAttribute(Qt::WA_NoBackground);
    ui->label_video_full->setAttribute(Qt::WA_NoBackground);

    mSettings = Settings::getInstant();
    QString path = mSettings->getApplicationPath() + "/config.ini";
    qDebug() << "path " << path;
    mSettings->loadSettings(path);

    mCurVideoChannel = VIDEO_CHANNEL_FRONT;
    connect(&mVideoUpdateTimer, SIGNAL(timeout()), this, SLOT(onUpdate()));
    connect(&mVideoUpdateSmallTimer, SIGNAL(timeout()), this, SLOT(onUpdateSmallImage()));
    connect(&mVideoUpdateFullTimer, SIGNAL(timeout()), this, SLOT(onUpdateFullImage()));
    connect(ui->pb_front, SIGNAL(clicked()), this, SLOT(onClickFront()));
    connect(ui->pb_rear, SIGNAL(clicked()), this, SLOT(onClickRear()));
    connect(ui->pb_left, SIGNAL(clicked()), this, SLOT(onClickLeft()));
    connect(ui->pb_right, SIGNAL(clicked()), this, SLOT(onClickRight()));

    mFPS = mSettings->mFps;

    mLastUpdateSmall = 0.0;
    mLastUpdateFull = 0.0;

    mController.init(Settings::getInstant()->mVideoChanel, VIDEO_CHANNEL_SIZE);
    start();
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
    mVideoUpdateTimer.start(1000/mFPS);
    int pano2DWidth = Settings::getInstant()->mPano2DWidth;
    int pano2DHeight = Settings::getInstant()->mPano2DHeight;
    bool enableOpenCL = (Settings::getInstant()->mEnableOpenCL == 1);

    QString path = Settings::getInstant()->getApplicationPath() + "/PanoConfig.bin";

    mController.start(mFPS,
			pano2DWidth,
			pano2DHeight,
			(char*)path.toStdString().c_str(),
			enableOpenCL);
}

void MainWindow::stop()
{
    mController.stop();
}

void MainWindow::onControllerQuit()
{
    mController.uninit();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    updateFullImage();
    updateSmallImage();
}

void MainWindow::onUpdate()
{
    update();
}

void MainWindow::updateFullImage()
{
#if DEBUG_UPDATE
    double timestamp = 0.0;
    double start = (double)clock();
    double showElapsed = 0;
    if (qAbs(mLastUpdateFull) > 0.00001f)
    {
        showElapsed = (int)(start - mLastUpdateFull)/CLOCKS_PER_SEC;
    }
    mLastUpdateFull = start;
#endif
    surround_image_t* surroundImage = mController.dequeuePano2DImage();
#if DEBUG_UPDATE
    double end = (double)clock();
#endif

    if (NULL == surroundImage)
    {
        return;
    }

    double elapsed = (int)((double)clock() - surroundImage->timestamp)/CLOCKS_PER_SEC;
#if DEBUG_UPDATE
    timestamp = surroundImage->timestamp;
    double start1 = (double)clock();
#endif

    //if (elapsed < 1000/mUpdateFPS)
    {
        cv::Mat* frame = (cv::Mat*)(surroundImage->frame.data);
        QImage image = cvMat2QImage(*frame);
        ui->label_video_full->setPixmap(QPixmap::fromImage(image));
        //QPainter painter(this);
        //painter.drawImage(QPoint(20,20), image);
        delete frame;
    }
    delete surroundImage;

#if DEBUG_UPDATE
    double end1 = (double)clock();

    qDebug() << "MainWindow::onUpdateFullImage"
             << ", fps:" << mFPS
             << ", elapsed to last update:" << showElapsed
             << ", timestamp:" << timestamp
             << ", elapsed to capture:" << elapsed
             << ", read:" << (end-start)/CLOCKS_PER_SEC
             << ", show:" << (end1-start1)/CLOCKS_PER_SEC;

#endif
}

void MainWindow::updateSmallImage()
{
#if DEBUG_UPDATE
    double timestamp = 0.0;
    double start = (double)clock();
    double showElapsed = 0;
    if (qAbs(mLastUpdateSmall) > 0.00001f)
    {
        showElapsed = (start - mLastUpdateSmall)/CLOCKS_PER_SEC;
    }
    mLastUpdateSmall = start;
#endif
    surround_image_t* surroundImage = mController.dequeueSideImage(mCurVideoChannel);
#if DEBUG_UPDATE
    double end = (double)clock();
#endif

    if (NULL == surroundImage)
    {
        return;
    }

    double elapsed = (clock() - surroundImage->timestamp)/CLOCKS_PER_SEC;
#if DEBUG_UPDATE
    timestamp = surroundImage->timestamp;
    double start1 = (double)clock();
#endif

    //if (elapsed < 1000/mUpdateFPS)
    {
        cv::Mat* frame = (cv::Mat*)(surroundImage->frame.data);
        QImage image = cvMat2QImage(*frame);
        ui->label_video_small->setPixmap(QPixmap::fromImage(image));
        delete frame;
    }
    delete surroundImage;

#if DEBUG_UPDATE
    double end1 = (double)clock();
    qDebug() << "MainWindow::onUpdateSmallImage"
             << ", fps:" << mFPS
             << ", elapsed to last update:" << showElapsed
             << ", timestamp:" << timestamp
             << ", elapsed to capture:" << elapsed
             << ", read:" << (end-start)/CLOCKS_PER_SEC
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
