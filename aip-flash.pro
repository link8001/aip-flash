#-------------------------------------------------
#
# Project created by QtCreator 2016-09-07T16:09:28
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = aip-flash
TEMPLATE = app


SOURCES += main.cpp\
        CWinFlash.cpp

HEADERS  += CWinFlash.h

FORMS    += CWinFlash.ui

RESOURCES += \
    image.qrc

RC_FILE += ico.rc
