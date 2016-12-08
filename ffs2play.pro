#-------------------------------------------------
#
# Project created by QtCreator 2016-09-11T20:18:08
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ffs2play
TEMPLATE = app

INCLUDEPATH +=	./include \
                ./src \
                ./forms

SOURCES +=	src/main.cpp \
                src/gui/mainwindow.cpp \
                src/net/udpserver.cpp
				
HEADERS  +=     include/gui/mainwindow.h \
                include/net/udpserver.h

FORMS    +=     ./forms/mainwindow.ui

RESOURCES += \
    ressources.qrc
CONFIG += static
