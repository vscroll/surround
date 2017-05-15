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
        settings.cpp \
        ../../util/util.cpp \
        ../../util/shm/shmutil.cpp \
        ../imageshm/imageshm.cpp

HEADERS  += mainwindow.h \
        settings.h \
        ../../include/common.h \
        ../../util/util.h \
        ../../util/shm/shmutil.h \
        ../imageshm/imageshm.h

FORMS    += mainwindow.ui
