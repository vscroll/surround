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
    capture1worker.cpp \
    capture4worker.cpp \
    capture4impl.cpp \
    capture1impl.cpp \
    controller.cpp \
    stitchworker.cpp \
    stitchimpl.cpp \
    stitch_algorithm.cpp \
    util.cpp

HEADERS  += mainwindow.h \
    capture1worker.h \
    common.h \
    capture4worker.h \
    ICapture.h \
    capture4impl.h \
    capture1impl.h \
    controller.h \
    stitchworker.h \
    IStitch.h \
    stitchimpl.h \
    stitch_algorithm.h \
    util.h

FORMS    += mainwindow.ui
