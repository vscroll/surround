#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    mCurVideoChannel = VIDEO_CHANNEL_FRONT;
    connect(&mVideoUpdateSmallTimer, SIGNAL(timeout()), this, SLOT(onUpdateSmallImage()));
    connect(&mVideoUpdateFullTimer, SIGNAL(timeout()), this, SLOT(onUpdateFullImage()));
    connect(ui->pb_front, SIGNAL(clicked()), this, SLOT(onClickFront()));
    connect(ui->pb_rear, SIGNAL(clicked()), this, SLOT(onClickRear()));
    connect(ui->pb_left, SIGNAL(clicked()), this, SLOT(onClickLeft()));
    connect(ui->pb_right, SIGNAL(clicked()), this, SLOT(onClickRight()));

    mUpdateFPS = VIDEO_FPS_15;
    mCaptureFPS = VIDEO_FPS_15;

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
    mVideoUpdateSmallTimer.start(1000/mUpdateFPS);
    mVideoUpdateFullTimer.start(1000/mUpdateFPS);
    QObject::connect(&mController, SIGNAL(finished()), this, SLOT(onControllerQuit()));
    mController.start(mCaptureFPS);
}

void MainWindow::stop()
{
    mController.stop();
}

void MainWindow::onControllerQuit()
{
    mController.uninit();
}

void MainWindow::onUpdateSmallImage()
{

#if DEBUG
    double start = (double)clock();
#endif
    surround_image1_t* surroundImage = mController.dequeueSmallImage(mCurVideoChannel);
#if DEBUG
    double end = (double)clock();
#endif

    if (NULL == surroundImage)
    {
        return;
    }

    int elapsed = (int)((double)clock() - surroundImage->timestamp)/1000;
#if DEBUG
    double start1 = (double)clock();
#endif

    //if (elapsed < 1000/mUpdateFPS)
    {
        IplImage* frame = (IplImage*)(surroundImage->image);
        QImage image((const uchar*)frame->imageData,
                     frame->width,
                     frame->height,
                     QImage::Format_RGB888);
         ui->label_video_small->setPixmap(QPixmap::fromImage(image));
    }

#if DEBUG
    double end1 = (double)clock();
    qDebug() << "MainWindow::onUpdateSmallImage"
             << ", fps:" << mUpdateFPS
             << ", elapsed to capture:" << elapsed
             << ", read:" << (int)(end-start)/1000
             << ", show:" << (int)(end1-start1)/1000;
#endif

    cvReleaseImage((IplImage**)(&(surroundImage->image)));
    delete surroundImage;

}


void MainWindow::onUpdateFullImage()
{
#if DEBUG
    double start = (double)clock();
#endif
    surround_image1_t* surroundImage = mController.dequeueFullImage();
#if DEBUG
    double end = (double)clock();
#endif

    if (NULL == surroundImage)
    {
        return;
    }

    int elapsed = (int)((double)clock() - surroundImage->timestamp)/1000;
#if DEBUG
    double start1 = (double)clock();
#endif

    //if (elapsed < 1000/mUpdateFPS)
    {
        IplImage* frame = (IplImage*)(surroundImage->image);
        QImage image((const uchar*)frame->imageData,
                     frame->width,
                     frame->height,
                     QImage::Format_RGB888);
         ui->label_video_full->setPixmap(QPixmap::fromImage(image));
    }

#if DEBUG
    double end1 = (double)clock();
    qDebug() << "MainWindow::onUpdateFullImage"
             << ", fps:" << mUpdateFPS
             << ", elapsed to capture:" << elapsed
             << ", read:" << (int)(end-start)/1000
             << ", show:" << (int)(end1-start1)/1000;
#endif
    cvReleaseImage((IplImage**)(&(surroundImage->image)));
    delete surroundImage;
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
