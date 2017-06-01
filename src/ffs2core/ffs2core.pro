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
OPENSSL_INCLUDE_PATH = "E:\devel\openssl-1.0.2l\inc32"
OPENSSL_LIB_PATH = "E:\devel\openssl-1.0.2l\out32dll"
CRYPTOPP_INCLUDE_PATH = "E:\devel\git\cryptopp"
CONFIG( debug, debug|release ) {
    CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\build\Debug"
    MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\msvc\Debug
} else {
    CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\build\Release"
    MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\msvc\Release
}

INCLUDEPATH +=	$${MINIUPNPC_PATH} \
                $${OPENSSL_INCLUDE_PATH} \
                $${CRYPTOPP_INCLUDE_PATH} \
                $${PROTOBUF_PATH}

SOURCES +=      net/udpserver.cpp \
                singleton/clogger.cpp \
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
                singleton/simbasemanager.cpp \
                sim/simconnector.cpp

HEADERS  +=     net/udpserver.h \
                net/chttprequest.h \
                protobuf/ffs2play.pb.h \
                singleton/clogger.h \
                singleton/call_once.h \
                singleton/p2pmanager.h \
                singleton/singleton.h \
                singleton/webmanager.h \
                singleton/simbasemanager.h \
                tools/version.h \
                tools/getappversion.h \
                tools/crypto.h \
                tools/aircraftstate.h \
                tools/ffs2core_global.h \
                sim/simconnector.h

TRANSLATIONS += ../../intl/ffs2core_fr_FR.ts

DISTFILES += \
    ../../intl/ffs2core_fr_FR.ts


#Libs Path
LIBS    += -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.15063.0\um\x86"
LIBS	+= -L$${OPENSSL_LIB_PATH}
LIBS	+= -L$${MINIUPNPC_LIB_PATH}
LIBS	+= -L$${CRYPTOPP_LIB_PATH}
LIBS	+= -L$${PROTOBUF_PATH}/lib

# Libs to load
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

