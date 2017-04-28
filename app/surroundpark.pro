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
        settings.cpp

HEADERS  += mainwindow.h \
        settings.h

FORMS    += mainwindow.ui
