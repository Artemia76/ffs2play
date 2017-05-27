#-------------------------------------------------
#
# Project created by QtCreator 2016-09-11T20:18:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += network xml

TARGET = ffs2core
TEMPLATE = lib
DEFINES += MINIUPNP_STATICLIB
DEFINES += FFS2CORE_DLL

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

INCLUDEPATH +=	"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\inc" \
                $${MINIUPNPC_PATH} \
                $${OPENSSL_INCLUDE_PATH} \
                $${CRYPTOPP_INCLUDE_PATH} \
                $${PROTOBUF_PATH}

SOURCES +=      net/udpserver.cpp \
                singleton/clogger.cpp \
                singleton/scmanager.cpp \
                net/chttprequest.cpp \
                singleton/p2pmanager.cpp \
                singleton/webmanager.cpp \
                tools/getappversion.cpp \
                tools/crypto.cpp \
                protobuf/ffs2play.pb.cc \
                singleton/webmanager.Requests.cpp \
                singleton/webmanager.Callback.cpp \
                singleton/webmanager.Utils.cpp \
                tools/aircraftstate.cpp \
                singleton/simbasemanager.cpp

HEADERS  +=     net/udpserver.h \
                net/chttprequest.h \
                protobuf/ffs2play.pb.h \
                singleton/clogger.h \
                singleton/call_once.h \
                singleton/p2pmanager.h \
                singleton/singleton.h \
                singleton/webmanager.h \
                singleton/scmanager.h \
                singleton/simbasemanager.h \
                tools/version.h \
                tools/getappversion.h \
                tools/crypto.h \
                tools/aircraftstate.h \
                tools/ffs2core_global.h

TRANSLATIONS += ../intl/ffs2play_fr_FR.ts

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

# Application Version

VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_BUILD = 0
DEFINES += APP_NAME=\\\"$${TARGET}\\\"
DEFINES += APP_MAJOR=$$VERSION_MAJOR
DEFINES += APP_MINOR=$$VERSION_MINOR
DEFINES += APP_BUILD=$$VERSION_BUILD

