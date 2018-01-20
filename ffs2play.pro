#-------------------------------------------------
#
# Project created by QtCreator 2016-09-11T20:18:08
#
#-------------------------------------------------

QT       += core gui

TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network xml

TARGET = ffs2play
DEFINES += MINIUPNP_STATICLIB

win32 {
DEFINES += FFS_WITH_SC
}

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
	DEFINES += FFS_DEBUG
}

win32 {
	MINIUPNPC_PATH = "E:\devel\git\miniupnp\miniupnpc"
	OPENSSL_INCLUDE_PATH = "E:\devel\openssl-1.0.2l\inc32"
	OPENSSL_LIB_PATH = "E:\devel\openssl-1.0.2l\out32dll"
	CRYPTOPP_INCLUDE_PATH = "E:\devel\git\cryptopp"
CONFIG( debug, debug|release ) {
	CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\Win32\Output\Debug"
	MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\msvc\Debug
} else {
	CRYPTOPP_LIB_PATH = "E:\devel\git\cryptopp\Win32\Output\Release"
	MINIUPNPC_LIB_PATH = $${MINIUPNPC_PATH}\msvc\Release
}
	PROTOBUF_LIB_PATH = "D:\Program Files (x86)\protobuf\lib"
	PROTOBUF_INCLUDE_PATH = "D:\Program Files (x86)\protobuf\include"

	INCLUDEPATH +=	\
				"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\inc" \
#				"D:\Program Files (x86)\Lockheed Martin\Prepar3D v3 SDK 3.4.9.18400\Utilities\SimConnect SDK\Inc" \
				$${MINIUPNPC_PATH} \
				$${OPENSSL_INCLUDE_PATH} \
				$${CRYPTOPP_INCLUDE_PATH} \
				$${PROTOBUF_INCLUDE_PATH} \
}

unix{
	INCLUDEPATH += /usr/include/miniupnpc \
					/usr/include/cryptopp
}

UI_DIR =		gui

SOURCES +=		main.cpp \
				gui/dialogprofils.cpp \
				gui/mainwindow.cpp \
				net/chttprequest.cpp \
				net/peer.cpp \
                net/peer.Event.Cpp \
                net/peer.Requests.cpp \
				net/udpserver.cpp \
				protobuf/ffs2play.pb.cc \
				sim/aibase.cpp \
				sim/aircraftstate.cpp \
				sim/simconnector.cpp \
                sim/scmanager.cpp \
                sim/xpmanager.cpp \
				singleton/aimapping.cpp \
				singleton/clogger.cpp \
				singleton/p2pmanager.cpp \
				singleton/simbasemanager.cpp \
				singleton/webmanager.Callback.cpp \
				singleton/webmanager.cpp \
				singleton/webmanager.Requests.cpp \
				singleton/webmanager.Utils.cpp \
				tools/crypto.cpp \
				tools/getappversion.cpp \
                tools/datetime.cpp \
                tools/calculation.cpp

HEADERS  +=     gui/dialogprofils.h \
				gui/mainwindow.h \
				net/chttprequest.h \
				net/peer.h \
				net/udpserver.h \
				protobuf/ffs2play.pb.h \
				sim/aibase.h \
				sim/aircraftstate.h \
				sim/simconnector.h \
                sim/scmanager.h \
				sim/xpmanager.h \
				singleton/aimapping.h \
				singleton/call_once.h \
				singleton/clogger.h \
				singleton/p2pmanager.h \
				singleton/simbasemanager.h \
				singleton/singleton.h \
				singleton/webmanager.h \
				tools/crypto.h \
				tools/datetime.h \
				tools/getappversion.h \
                tools/version.h \
                tools/calculation.h

FORMS	+=		gui/dialogprofils.ui \
				gui/mainwindow.ui

TRANSLATIONS += intl/ffs2play_fr_FR.ts

trans.path = $${DESTDIR}/translations
trans.files = intl/*.qm
INSTALLS += trans

win32 {
	#Libs path
	LIBS	+= -L"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.15063.0\um\x86"
	LIBS	+= -L"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\lib"
	#LIBS	+= -L"D:\Program Files (x86)\Lockheed Martin\Prepar3D v3 SDK 3.4.9.18400\Utilities\SimConnect SDK\lib"
	LIBS	+= -L$${OPENSSL_LIB_PATH}
	LIBS	+= -L$${MINIUPNPC_LIB_PATH}
	LIBS	+= -L$${CRYPTOPP_LIB_PATH}
	LIBS	+= -L$${PROTOBUF_LIB_PATH}

	#Libs to load
	LIBS	+= -lSimConnect
	LIBS	+= -lWs2_32
	LIBS	+= -liphlpapi
	LIBS	+= -lminiupnpc
	LIBS	+= -lcryptlib
	LIBS	+= -llibeay32
CONFIG( debug, debug|release ) {
	LIBS	+= -llibprotobufd
} else {
	LIBS	+= -llibprotobuf
}

RC_FILE = ffs2play.rc
}

RESOURCES += ressources.qrc

unix {
CONFIG += link_pkgconfig
PKGCONFIG += protobuf
PKGCONFIG += libcrypto++

LIBS += -lminiupnpc
LIBS += -lssl
LIBS += -lcrypto
}

win {
	DISTFILES += ffs2play.rc
}

DISTFILES += intl/ffs2play_fr_FR.ts \
	README.md
DISTFILES += protobuf/ffs2play.proto


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
# Application Version

VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_BUILD = 0
DEFINES += APP_NAME=\\\"$${TARGET}\\\"
DEFINES += APP_MAJOR=$$VERSION_MAJOR
DEFINES += APP_MINOR=$$VERSION_MINOR
DEFINES += APP_BUILD=$$VERSION_BUILD

