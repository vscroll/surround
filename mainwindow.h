#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include "controller.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void start();
    void stop();

private slots:
    void onUpdateFullImage();
    void onUpdateSmallImage();
    void onClickFront();
    void onClickRear();
    void onClickLeft();
    void onClickRight();
    void onControllerQuit();
private:
    Ui::MainWindow *ui;
    QLabel *mVideoLabelFull;
    QLabel *mVideoLabelSmall;

    QTimer mVideoUpdateFullTimer;
    QTimer mVideoUpdateSmallTimer;

    VIDEO_CHANNEL mCurVideoChannel;
    VIDEO_FPS mUpdateFPS;

    VIDEO_FPS mCaptureFPS;
    Controller mController;
};

#endif // MAINWINDOW_H

