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
                $${PROTOBUF_PATH} \
                ../ffs2core

SOURCES +=	main.cpp \
                mainwindow.cpp \
                dialogprofils.cpp \
				
HEADERS  +=     mainwindow.h \
                dialogprofils.h \

FORMS	+=	mainwindow.ui \
                dialogprofils.ui

TRANSLATIONS += ../../intl/ffs2play_fr_FR.ts

#Libs Path
CONFIG( debug, debug|release ) {
LIBS	+= -L../ffs2core/debug -lffs2core
} else {
LIBS	+= -L../ffs2core/release -lffs2core
}

RESOURCES += ressources.qrc

RC_FILE = ffs2play.rc

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
    ../../intl/ffs2play_fr_FR.ts

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
