#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include "controller.h"
#include <opencv/cv.h>

namespace Ui {
class MainWindow;
}

class Settings;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void start();
    void stop();

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);

private slots:
    void onUpdate();
    void onUpdateFullImage();
    void onUpdateSmallImage();
    void onClickFront();
    void onClickRear();
    void onClickLeft();
    void onClickRight();
    void onControllerQuit();

private:
    void updateFullImage();
    void updateSmallImage();
    QImage cvMat2QImage(const cv::Mat& mat);
private:
    Ui::MainWindow *ui;
    QLabel *mVideoLabelFull;
    QLabel *mVideoLabelSmall;

    Settings* mSettings;

    QTimer mVideoUpdateTimer;

    QTimer mVideoUpdateFullTimer;
    QTimer mVideoUpdateSmallTimer;

    unsigned int mCurVideoChannel;

    int mFPS;
    Controller mController;

    double mLastUpdateSmall;
    double mLastUpdateFull;
};

#endif // MAINWINDOW_H

