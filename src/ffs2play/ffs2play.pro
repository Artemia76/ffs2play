#-------------------------------------------------
#
# Project created by QtCreator 2016-09-11T20:18:08
#
#-------------------------------------------------

QT       += core gui

TEMPLATE = app

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network xml

TARGET = ffs2play
DEPENDPATH += ../ffs2core

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

INCLUDEPATH +=	\
                "D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\inc" \
 #               "D:\Program Files (x86)\Lockheed Martin\Prepar3D v3 SDK 3.4.9.18400\Utilities\SimConnect SDK\Inc" \
                $${MINIUPNPC_PATH} \
                $${OPENSSL_INCLUDE_PATH} \
                $${CRYPTOPP_INCLUDE_PATH} \
                $${PROTOBUF_PATH} \
                ../ffs2core

SOURCES +=	main.cpp \
                mainwindow.cpp \
                dialogprofils.cpp \
                scmanager.cpp
				
HEADERS  +=     mainwindow.h \
                dialogprofils.h \
                scmanager.h

FORMS	+=	mainwindow.ui \
                dialogprofils.ui

TRANSLATIONS += ../../intl/ffs2play_fr_FR.ts

#Libs Path
CONFIG( debug, debug|release ) {
LIBS	+= -L../ffs2core/debug -lffs2core
} else {
LIBS	+= -L../ffs2core/release -lffs2core
}
LIBS	+= -L"D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK\lib"
#LIBS	+= -L"D:\Program Files (x86)\Lockheed Martin\Prepar3D v3 SDK 3.4.9.18400\Utilities\SimConnect SDK\lib"
LIBS	+= -lSimConnect

RESOURCES += ressources.qrc

RC_FILE = ffs2play.rc

DISTFILES += \
    ffs2play.rc \
    ../../intl/ffs2play_fr_FR.ts


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
    DEPLOY_COMMAND = windeployqt --no-system-d3d-compiler --no-opengl-sw --no-compiler-runtime
}
macx {
    DEPLOY_COMMAND = macdeployqt
}

CONFIG( debug, debug|release ) {
    # debug
    DEPLOY_PATH = $${OUT_PWD}/debug
    DEPLOY_TARGET = $$shell_quote($$shell_path($${DEPLOY_PATH}/$${TARGET}$${TARGET_CUSTOM_EXT}))
    DEPLOY_TARGET += $$shell_quote($$shell_path($${OUT_PWD}/../ffs2core/debug/ffs2core.dll))
} else {
    # release
    DEPLOY_PATH = $${OUT_PWD}/release
    DEPLOY_TARGET = $$shell_quote($$shell_path($${DEPLOY_PATH}/$${TARGET}$${TARGET_CUSTOM_EXT}))
    DEPLOY_TARGET += $$shell_quote($$shell_path($${OUT_PWD}/../ffs2core/release/ffs2core.dll))
}

#  # Uncomment the following line to help debug the deploy command when running qmake
  warning($${DEPLOY_COMMAND} $${DEPLOY_TARGET})

# Use += instead of = if you use multiple QMAKE_POST_LINKs
QMAKE_POST_LINK += $$quote($${DEPLOY_COMMAND} $${DEPLOY_TARGET}$$escape_expand(\n\t))

#copy depends DLLs
win32 {
    EXTRA_DLLS += $${OPENSSL_LIB_PATH}/libeay32.dll
CONFIG( debug, debug|release ) {
    # debug
    EXTRA_DLLS += $${OUT_PWD}/../ffs2core/debug/ffs2core.dll
} else {
    # release
    EXTRA_DLLS += $${OUT_PWD}/../ffs2core/release/ffs2core.dll
}
    EXTRA_DLLS ~= s,/,\\,g
    DESTDIR_DLLS = $${DEPLOY_PATH}
    DESTDIR_DLLS ~= s,/,\\,g
    for(FILE,EXTRA_DLLS){
                QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_DLLS}$$escape_expand(\n\t))
    }
}

#install language files
win32 {
    EXTRA_LANGFILES += $${OUT_PWD}/../../intl/ffs2play_fr_FR.qm
    EXTRA_LANGFILES += $${OUT_PWD}/../../intl/ffs2core_fr_FR.qm
    EXTRA_LANGFILES ~= s,/,\\,g
    DESTDIR_TRANSLATION = $${DEPLOY_PATH}/translations
    DESTDIR_TRANSLATION ~= s,/,\\,g
    for(FILE,EXTRA_LANGFILES){
                QMAKE_POST_LINK +=$$quote(cmd /c copy /y $${FILE} $${DESTDIR_TRANSLATION}$$escape_expand(\n\t))
    }
}

warning($${QMAKE_POST_LINK})

# Application Version

VERSION_MAJOR = 2
VERSION_MINOR = 0
VERSION_BUILD = 0
DEFINES += APP_NAME=\\\"$${TARGET}\\\"
DEFINES += APP_MAJOR=$$VERSION_MAJOR
DEFINES += APP_MINOR=$$VERSION_MINOR
DEFINES += APP_BUILD=$$VERSION_BUILD

