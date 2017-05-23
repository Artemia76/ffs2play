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
PROTOBUF_PATH = "E:\devel\protobuf"
OPENSSL_INCLUDE_PATH = "D:\OpenSSL-Win32-1-0-2\include"
OPENSSL_LIB_PATH = "D:\OpenSSL-Win32-1-0-2\lib"
CRYPTOPP_INCLUDE_PATH = "E:\devel\git\cryptopp"
CONFIG( debug, debug|release ) {
    CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\build\Debug"
    MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\msvc\Debug
} else {
    CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\build\Release"
    MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\msvc\Release
}

INCLUDEPATH +=	./include \
                ./src \
                ./forms \
                "D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\inc" \
                $${MINIUPNPC_PATH} \
                $${OPENSSL_INCLUDE_PATH} \
                $${CRYPTOPP_INCLUDE_PATH} \
                $${PROTOBUF_PATH}

SOURCES +=	src/main.cpp \
                src/gui/mainwindow.cpp \
                src/net/udpserver.cpp \
                src/singleton/clogger.cpp \
                src/singleton/scmanager.cpp \
                src/net/chttprequest.cpp \
                src/singleton/p2pmanager.cpp \
                src/singleton/webmanager.cpp \
                src/gui/dialogprofils.cpp \
                src/tools/getappversion.cpp \
                src/tools/crypto.cpp \
                protobuf/ffs2play.pb.cc \
    src/singleton/webmanager.Requests.cpp \
    src/singleton/webmanager.Callback.cpp \
    src/singleton/webmanager.Utils.cpp \
    src/tools/aircraftstate.cpp
				
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
                include/tools/getappversion.h \
                include/tools/crypto.h \
                protobuf/ffs2play.pb.h \
    include/tools/aircraftstate.h

FORMS	+=	forms/mainwindow.ui \
                forms/dialogprofils.ui

TRANSLATIONS += intl/ffs2play_fr_FR.ts

DEFINES += MINIUPNP_STATICLIB
#Libs Path
LIBS    += -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.15063.0\um\x86"
LIBS	+= -L"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\lib"
LIBS	+= -L$${OPENSSL_LIB_PATH}
LIBS	+= -L$${MINIUPNPC_LIB_PATH}
LIBS	+= -L$${CRYPTOPP_LIB_PATH}
LIBS	+= -L$${PROTOBUF_PATH}/lib

# Libs to load
LIBS	+= -lSimConnect
LIBS	+= -lWs2_32
LIBS	+= -liphlpapi
LIBS	+= -lminiupnpc
LIBS	+= -lcryptopp-static
LIBS	+= -llibeay32
CONFIG( debug, debug|release ) {
LIBS	+= -llibprotobufd
} else {
LIBS	+= -llibprotobuf
}

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
    DEPLOY_PATH = $${OUT_PWD}/debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($${DEPLOY_PATH}/$${TARGET}$${TARGET_CUSTOM_EXT}))
} else {
    # release
    DEPLOY_PATH = $${OUT_PWD}/release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${DEPLOY_PATH}/$${TARGET}$${TARGET_CUSTOM_EXT}))
}

#  # Uncomment the following line to help debug the deploy command when running qmake
#  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
QMAKE_POST_LINK = $$quote($${DEPLOY_COMMAND} $${DEPLOY_TARGET}$$escape_expand(\n\t))

DISTFILES += \
    ffs2play.rc \
    intl/ffs2play_fr_FR.ts

# Application Version

VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_BUILD = 0
DEFINES += APP_NAME=\\\"$${TARGET}\\\"
DEFINES += APP_MAJOR=$$VERSION_MAJOR
DEFINES += APP_MINOR=$$VERSION_MINOR
DEFINES += APP_BUILD=$$VERSION_BUILD

win32 {
    EXTRA_LANGFILES = $${OUT_PWD}/intl/ffs2play_fr_FR.qm
    EXTRA_LANGFILES ~= s,/,\\,g
    DESTDIR_WIN = $${DEPLOY_PATH}/translations
    DESTDIR_WIN ~= s,/,\\,g
    for(FILE,EXTRA_LANGFILES){
                QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
    }
}
