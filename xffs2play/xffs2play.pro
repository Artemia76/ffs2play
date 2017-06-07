#-------------------------------------------------
#
# Project created by QtCreator 2017-06-07T18:15:54
#
#-------------------------------------------------

QT += core

TEMPLATE = lib
DEPENDPATH += ../ffs2core
CONFIG += warn_on release shared
#CONFIG -= rtti exceptions thread qt
DEFINES += XFFS2PLAY_LIBRARY FFS_DEBUG

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG(release, debug|release) {
    DESTDIR = release
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = release/.ui
} else {
    DESTDIR = debug
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = debug/.ui
}

win32 {
    TARGET = win.xpl
    DEFINES += APL=0 IBM=1 LIN=0 WIN_32

    XPSDK_FOLDER = E:\devel\XPSDK213\SDK
    LIBS += -L$${XPSDK_FOLDER}/Libraries/Win
    LIBS += -lXPLM -lXPWidgets
}
unix {
    TARGET = lin.xpl
    DEFINES += APL=0 IBM=0 LIN=1
    QMAKE_CFLAGS += -fstack-protector
    QMAKE_CXXFLAGS += -fstack-protector
    XPSDK_FOLDER = /home/gianni/projet/XPSDK213/SDK
}
macx {
    TARGET = mac.xpl
    DEFINES += APL=1 IBM=0 LIN=0
    QMAKE_LFLAGS += -flat_namespace -undefined suppress

    # Build for multiple architectures.
    # The following line is only needed to build universal on PPC architectures.
    # QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.4u.sdk
    # This line defines for which architectures we build.
    CONFIG += x86 ppc
}
INCLUDEPATH += $${XPSDK_FOLDER}/CHeaders/XPLM
#INCLUDEPATH += $${XPSDK_FOLDER}/CHeaders/Wrappers
INCLUDEPATH += $${XPSDK_FOLDER}/CHeaders/Widgets

INCLUDEPATH += ../ffs2core

SOURCES += \
        xffs2play.cpp \
        MainFrame.cpp

HEADERS += \
        xffs2play_global.h \ 
        MainFrame.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
