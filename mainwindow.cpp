#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QPainter>
#include <QDir>
#include <QDebug>
#include "util.h"
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

    mController.init();
    start();
}

MainWindow::~MainWindow()
{    
    stop();

    delete ui;
}

void MainWindow::start()
{
    mVideoUpdateTimer.start(1000/mFPS);
    QObject::connect(&mController, SIGNAL(finished()), this, SLOT(onControllerQuit()));
    mController.start(mFPS);
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
    surround_image_t* surroundImage = mController.dequeueFullImage();
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
        QImage image = Util::cvMat2QImage(*frame);
        ui->label_video_full->setPixmap(QPixmap::fromImage(image));
        //QPainter painter(this);
        //painter.drawImage(QPoint(20,20), image);
        delete frame;
    }
    delete surroundImage;

#if DEBUG_UPDATE
    double end1 = (double)clock();

    qDebug() << "MainWindow::onUpdateFullImage"
             << " fps:" << mFPS
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
    surround_image_t* surroundImage = mController.dequeueSmallImage(mCurVideoChannel);
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
        QImage image = Util::cvMat2QImage(*frame);
        ui->label_video_small->setPixmap(QPixmap::fromImage(image));
        delete frame;
    }
    delete surroundImage;

#if DEBUG_UPDATE
    double end1 = (double)clock();
    qDebug() << "MainWindow::onUpdateSmallImage"
             << " fps:" << mFPS
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
