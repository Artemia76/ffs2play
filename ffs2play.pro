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
RC_FILE = ./ffs2play.rc

isEmpty(TARGET_EXT) {
    win32 {
        TARGET_CUSTOM_EXT = .exe
    }
    macx {
        TARGET_CUSTOM_EXT = .app
    }
} else {
    TARGET_CUSTOM_EXT = $${TARGET_EXT}
}

win32 {
    DEPLOY_COMMAND = windeployqt
}
macx {
    DEPLOY_COMMAND = macdeployqt
}

CONFIG( debug, debug|release ) {
    # debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/debug/$${TARGET}$${TARGET_CUSTOM_EXT}))
} else {
    # release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${OUT_PWD}/release/$${TARGET}$${TARGET_CUSTOM_EXT}))
}

#  # Uncomment the following line to help debug the deploy command when running qmake
#  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
QMAKE_POST_LINK = $${DEPLOY_COMMAND} $${DEPLOY_TARGET}
