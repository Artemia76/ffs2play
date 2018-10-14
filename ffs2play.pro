#-------------------------------------------------
#
# Project created by QtCreator 2016-09-11T20:18:08
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network xml

TARGET = ffs2play
TEMPLATE = app

DEFINES += QT_NO_CAST_FROM_BYTEARRAY
DEFINES += QT_NO_CAST_TO_ASCII

CONFIG(release, debug|release) {
    DESTDIR = release
    OBJECTS_DIR = release/.obj
    MOC_DIR = release/.moc
    RCC_DIR = release/.rcc
    UI_DIR = release/.ui
#   If debug info is needed for windows release to avoid
#   Install VS Studio Debug Env, just uncomment the line below
#   DEFINES += FFS_DEBUG
} else {
    DESTDIR = debug
    OBJECTS_DIR = debug/.obj
    MOC_DIR = debug/.moc
    RCC_DIR = debug/.rcc
    UI_DIR = debug/.ui
    DEFINES += FFS_DEBUG
}

win32 {
    MINIUPNPC_PATH = "E:\devel\git\miniupnp\miniupnpc"
    OPENSSL_INCLUDE_PATH = "E:\devel\openssl-1.0.2l\inc32"
    OPENSSL_LIB_PATH = "E:\devel\openssl-1.0.2l\out32dll"
    CRYPTOPP_INCLUDE_PATH = "E:\devel\git\cryptopp"
    DEFINES += _WINSOCKAPI_
    DEFINES += MINIUPNP_STATICLIB
CONFIG( debug, debug|release ) {
    CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\Win32\Output\Debug"
    MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\build\Debug
    } else {
    CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\Win32\Output\Release"
    MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\build\Release
    }
    PROTOBUF_LIB_PATH = "D:\Program Files (x86)\protobuf\lib"
    PROTOBUF_INCLUDE_PATH = "D:\Program Files (x86)\protobuf\include"

    EIGEN3_LIB_PATH = "D:\Program Files (x86)\protobuf\eigen3\lib"
    EIGEN3_INCLUDE_PATH = "D:\Program Files (x86)\eigen3\include"

    INCLUDEPATH +=	\
                "D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\inc" \
#               "D:\Program Files (x86)\Lockheed Martin\Prepar3D v3 SDK 3.4.9.18400\Utilities\SimConnect SDK\Inc" \
                $${MINIUPNPC_PATH} \
                $${OPENSSL_INCLUDE_PATH} \
                $${CRYPTOPP_INCLUDE_PATH} \
                $${PROTOBUF_INCLUDE_PATH} \
}

unix:!macx{
    INCLUDEPATH += /usr/include/miniupnpc \
                    /usr/include/cryptopp
}

macx: {
    INCLUDEPATH += /usr/local/include/miniupnpc
    INCLUDEPATH += /usr/local/include/cryptopp
    INCLUDEPATH += /usr/local/include
}

#Protobuf generation

PROTOS = $$files(protobuf/*.proto)
include(protobuf.pri)

UI_DIR = gui

SOURCES +=      main.cpp \
                gui/dialogprofils.cpp \
                gui/mainwindow.cpp \
                gui/mainwindow.GuiEvents.cpp \
                gui/dialogp2p.cpp \
                gui/aireplace.cpp \
                net/chttprequest.cpp \
                net/peer.cpp \
                net/peer.Requests.cpp \
                net/peer.Event.cpp \
                net/udpserver.cpp \
                sim/aircraftstate.cpp \
                sim/simconnector.cpp \
                sim/xpmanager.cpp \
                sim/fgmanager.cpp \
                singleton/aimapping.cpp \
                singleton/analyzer.cpp \
                singleton/clogger.cpp \
                singleton/p2pmanager.cpp \
                singleton/simbasemanager.cpp \
                singleton/webmanager.Callback.cpp \
                singleton/webmanager.cpp \
                singleton/webmanager.Requests.cpp \
                singleton/webmanager.Utils.cpp \
                tools/calculation.cpp \
                tools/crypto.cpp \
                tools/datagram.cpp \
                tools/datetime.cpp \
                tools/getappversion.cpp \
                tools/iniparser.cpp \
    sim/fgdatagram.cpp
win32 {
SOURCES +=      sim/scmanager.cpp
}

HEADERS  +=     gui/dialogprofils.h \
                gui/mainwindow.h \
                gui/dialogp2p.h \
                gui/aireplace.h \
                net/chttprequest.h \
                net/peer.h \
                net/udpserver.h \
                sim/aircraftstate.h \
                sim/simconnector.h \
                sim/xpmanager.h \
                sim/fgmanager.h \
                singleton/aimapping.h \
                singleton/analyzer.h \
                singleton/call_once.h \
                singleton/clogger.h \
                singleton/p2pmanager.h \
                singleton/simbasemanager.h \
                singleton/singleton.h \
                singleton/webmanager.h \
                tools/calculation.h \
                tools/crypto.h \
                tools/datagram.h \
                tools/datetime.h \
                tools/getappversion.h \
                tools/iniparser.h \
                tools/version.h \
    sim/fgdatagram.h

win32 {
HEADERS +=      sim/scmanager.h
}

FORMS +=        gui/aireplace.ui \
                gui/dialogp2p.ui \
                gui/dialogprofils.ui \
                gui/mainwindow.ui

TRANSLATIONS += intl/ffs2play_fr_FR.ts

trans.path = $${DESTDIR}/translations
trans.files = intl/*.qm
INSTALLS += trans

win32 {
    #Libs path
    LIBS += -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17134.0\um\x86"
    LIBS += -L"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\lib"
    #LIBS += -L"D:\Program Files (x86)\Lockheed Martin\Prepar3D v3 SDK 3.4.9.18400\Utilities\SimConnect SDK\lib"
    LIBS += -L$${OPENSSL_LIB_PATH}
    LIBS += -L$${MINIUPNPC_LIB_PATH}
    LIBS += -L$${CRYPTOPP_LIB_PATH}
    LIBS += -L$${PROTOBUF_LIB_PATH}

    #Libs to load
    LIBS += -lSimConnect
    LIBS += -lWs2_32
    LIBS += -liphlpapi
    LIBS += -lminiupnpc
    LIBS += -lcryptlib
    LIBS += -llibeay32
CONFIG( debug, debug|release ) {
    LIBS += -llibprotobufd
    } else {
    LIBS += -llibprotobuf
    }

    RC_FILE = ffs2play.rc
}

RESOURCES += ressources.qrc

CONFIG += c++14

unix:!macx{
CONFIG += link_pkgconfig
PKGCONFIG += protobuf

LIBS	+= -lminiupnpc
LIBS	+= -lssl
LIBS	+= -lcryptopp
LIBS	+= -lcrypto

}

macx{
QT_CONFIG -= no-pkg-config
CONFIG += link_pkgconfig

PKG_CONFIG = /usr/local/bin/pkg-config
PKGCONFIG += protobuf

LIBS += -L/usr/local/lib
LIBS += -lssl
LIBS += -lcryptopp
LIBS += -lcrypto
LIBS += -lminiupnpc

}

win {
    DISTFILES += ffs2play.rc
}

DISTFILES += \
    README.md \
    ffs2play.iss \
    style/ffs2play.qss


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
    DEPLOY_COMMAND = windeployqt --no-system-d3d-compiler --no-opengl --no-opengl-sw --no-angle --no-compiler-runtime
}
macx {
    DEPLOY_COMMAND = macdeployqt
}

win32 {
DEPLOY_TARGET = $$shell_quote($$shell_path($${DESTDIR}/$${TARGET}$${TARGET_CUSTOM_EXT}))
# Use += instead of = if you use multiple QMAKE_POST_LINKs
QMAKE_POST_LINK += $$quote($${DEPLOY_COMMAND} $${DEPLOY_TARGET}$$escape_expand(\n\t))
}

unix:!macx{
# Linux specific install
PREFIX = /usr

Binary.path = $${PREFIX}/bin
Binary.files = $${DESTDIR}/$${TARGET}
INSTALLS += Binary

Translation.path = $${PREFIX}/share/$${TARGET}/translations
Translation.files = $${DESTDIR}/translations/*.qm
INSTALLS += Translation

Theme.path = $${PREFIX}/share/$${TARGET}
Theme.files = $${DESTDIR}/*.qss
INSTALLS += Theme
}

#copy needed file to build directory

copydata.commands = $(COPY) $$shell_quote($$shell_path($$PWD/style/ffs2play.qss))  $$shell_quote($$shell_path($$PWD/$$DESTDIR))
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

# Application Version
VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_BUILD = 21
DEFINES += APP_NAME=\\\"$${TARGET}\\\"
DEFINES += APP_MAJOR=$$VERSION_MAJOR
DEFINES += APP_MINOR=$$VERSION_MINOR
DEFINES += APP_BUILD=$$VERSION_BUILD

QMAKE_EXTRA_TARGETS += versionTarget

#QMAKE_EXTRA_TARGETS += deploy
