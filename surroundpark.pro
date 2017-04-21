#-------------------------------------------------
#
# Project created by QtCreator 2017-03-15T17:04:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = surroundpark
TEMPLATE = app

LIBS += -lopencv_core -lopencv_highgui
SOURCES += main.cpp\
        mainwindow.cpp \
    capture4impl.cpp \
    capture1impl.cpp \
    controller.cpp \
    stitchworker.cpp \
    stitchimpl.cpp \
    stitch_algorithm.cpp \
    util.cpp \
    capture1workerv4l2impl.cpp \
    capture1workerbase.cpp \
    capture4workerbase.cpp \
    capture4workerimpl.cpp \
    capture1workerimpl.cpp \
    capture4workerv4l2impl.cpp \
    v4l2.cpp \
    imxipu.cpp \
    GoOnline.cpp \
    settings.cpp \
    stitch_cl.cpp

HEADERS  += mainwindow.h \
    common.h \
    ICapture.h \
    capture4impl.h \
    capture1impl.h \
    controller.h \
    stitchworker.h \
    IStitch.h \
    stitchimpl.h \
    stitch_algorithm.h \
    util.h \
    capture1workerv4l2impl.h \
    capture1workerbase.h \
    capture4workerbase.h \
    capture4workerimpl.h \
    capture1workerimpl.h \
    capture4workerv4l2impl.h \
    v4l2.h \
    imxipu.h \
    settings.h \
    stitch_cl.h

FORMS    += mainwindow.ui
