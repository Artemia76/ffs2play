#-------------------------------------------------
#
# Project created by QtCreator 2016-09-11T20:18:08
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network

TARGET = ffs2play
TEMPLATE = app

INCLUDEPATH +=	./include \
                ./src \
                ./forms \
                "D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\inc"

SOURCES +=	src/main.cpp \
                src/gui/mainwindow.cpp \
                src/net/udpserver.cpp \
                src/singleton/clogger.cpp \
                src/singleton/scmanager.cpp
				
HEADERS  +=     include/gui/mainwindow.h \
                include/net/udpserver.h \
                include/singleton/call_once.h \
                include/singleton/clogger.h \
                include/singleton/scmanager.h

FORMS    +=     ./forms/mainwindow.ui
LIBS    += -L"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\lib" -lSimConnect
RESOURCES += \
    ressources.qrc
