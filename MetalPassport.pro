#-------------------------------------------------
#
# Project created by QtCreator 2011-10-15T21:18:28
#
#-------------------------------------------------

QT       += core gui sql

TARGET = MetalPassport
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

#DEFINES += NO_SCANYAPY


# ScanyApi lib

win32: LIBS += -L"c:/Program Files/Cognitive/ScanifyAPI/libs" -lScApi

INCLUDEPATH += "c:/Program Files/Cognitive/ScanifyAPI/include"
DEPENDPATH += "c:/Program Files/Cognitive/ScanifyAPI/include"

win32: PRE_TARGETDEPS += "c:/Program Files/Cognitive/ScanifyAPI/libs/ScApi.lib"

RESOURCES += \
    Images.qrc
