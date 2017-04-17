#-------------------------------------------------
#
# Project created by QtCreator 2016-09-11T20:18:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network xml

TARGET = ffs2play
TEMPLATE = app

MINIUPNPC_PATH = "E:\devel\git\miniupnp\miniupnpc"
MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\msvc\Release

INCLUDEPATH +=	./include \
                ./src \
                ./forms \
                "D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\inc" \
                $${MINIUPNPC_PATH}

SOURCES +=	src/main.cpp \
                src/gui/mainwindow.cpp \
                src/net/udpserver.cpp \
                src/singleton/clogger.cpp \
                src/singleton/scmanager.cpp \
                src/net/chttprequest.cpp \
                src/singleton/p2pmanager.cpp \
                src/singleton/webmanager.cpp \
                src/gui/dialogprofils.cpp \
    src/tools/getappversion.cpp
				
HEADERS  +=     include/gui/mainwindow.h \
                include/net/udpserver.h \
                include/singleton/call_once.h \
                include/singleton/clogger.h \
                include/singleton/scmanager.h \
                include/net/chttprequest.h \
                include/singleton/p2pmanager.h \
                include/singleton/singleton.h \
                include/singleton/webmanager.h \
                include/gui/dialogprofils.h \
                include/tools/version.h \
    include/tools/getappversion.h

FORMS	+=	forms/mainwindow.ui \
                forms/dialogprofils.ui

LIBS    += -L"C:\Program Files (x86)\Windows Kits\8.1\Lib\winv6.3\um\x86"
LIBS	+= -L"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\lib"
LIBS	+= -L$${MINIUPNPC_LIB_PATH}
LIBS	+= -lSimConnect
LIBS	+= -lWs2_32
LIBS	+= -liphlpapi
LIBS	+= -lminiupnpc

RESOURCES += ressources.qrc

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
    DEPLOY_COMMAND = windeployqt --no-system-d3d-compiler --compiler-runtime
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

DISTFILES += \
    ffs2play.rc

# Application Version

VERSION_MAJOR = 1
VERSION_MINOR = 0
VERSION_BUILD = 0
DEFINES += APP_NAME=\\\"$${TARGET}\\\"
DEFINES += APP_MAJOR=$$VERSION_MAJOR
DEFINES += APP_MINOR=$$VERSION_MINOR
DEFINES += APP_BUILD=$$VERSION_BUILD
