#****************************************************************************
#
# Copyright (C) 2017 FSFranceSimulateur team.
# Contact: admin@ffs2play.fr
#
# FFS2Play is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# FFS2Play is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The license is as published by the Free Software
# Foundation and appearing in the file LICENSE.GPL3
# included in the packaging of this software. Please review the following
# information to ensure the GNU General Public License requirements will
# be met: https://www.gnu.org/licenses/gpl-3.0.html.
#****************************************************************************

QT += core widgets gui sql xml network

CONFIG += build_all c++14
CONFIG -= debug_and_release debug_and_release_target

TARGET = ffs2play
TEMPLATE = app

TARGET_NAME=ffs2play

TRANSLATIONS= \
    intl/ffs2play_fr_FR.ts \
    intl/ffs2play_es_ES.ts

# =======================================================================
# Copy ennvironment variables into qmake variables
SIMCONNECT_PATH=$$(SIMCONNECT_PATH)
DEPLOY_BASE=$$(DEPLOY_BASE)
GIT_PATH=$$(GIT_PATH)
MINIUPNPC_PATH=$$(MINIUPNPC_PATH)
MINIUPNPC_LIB_PATH=$$(MINIUPNPC_LIB_PATH)
OPENSSL_PATH=$$(OPENSSL_PATH)
OPENSSL_LIB_PATH=$$(OPENSSL_LIB_PATH)
CRYPTOPP_PATH=$$(CRYPTOPP_PATH)
CRYPTOPP_LIB_PATH=$$(CRYPTOPP_LIB_PATH)
WINDOWS_SDK_LIB_PATH=$$(WINDOWS_LIB_SDK_PATH)
PROTOBUF_PATH=$$(PROTOBUF_PATH)
PROTOBUF_LIB_PATH=$$(PROTOBUF_LIB_PATH)

# =======================================================================
# Fill defaults for unset
CONFIG(debug, debug|release) {
    CONF_TYPE=debug
    DEFINES += FFS_DEBUG=1
} else {
    CONF_TYPE=release
}

isEmpty(DEPLOY_BASE) : DEPLOY_BASE=$$PWD/../deploy

# =======================================================================
# Set compiler flags and paths

unix:!macx {
    isEmpty(GIT_PATH) : GIT_PATH=git

    # Find Miniupnpc location
    isEmpty(MINIUPNPC_PATH) {
        MINIUPNPC_PATH=/usr/include
    }
    INCLUDEPATH += $$MINIUPNPC_PATH/miniupnpc
    LIBS	+= -lminiupnpc

    # Find OpenSSL location
    exists( /lib/x86_64-linux-gnu/libssl.so.1.0.0 ) :  OPENSSL_PATH=/lib/x86_64-linux-gnu
    exists( /usr/lib/x86_64-linux-gnu/libssl.so.1.0.0 ) : OPENSSL_PATH=/usr/lib/x86_64-linux-gnu
    QMAKE_LFLAGS += -no-pie
    LIBS	+= -lssl

    # CryptoPP
    INCLUDEPATH += /usr/include/cryptopp
    LIBS	+= -lcryptopp
    LIBS	+= -lcrypto

    # Makes the shell script and setting LD_LIBRARY_PATH redundant
    QMAKE_RPATHDIR=.
    QMAKE_RPATHDIR+=./lib

    CONFIG += link_pkgconfig
    PKGCONFIG += protobuf
}

win32 {
    WINDEPLOY_FLAGS = --compiler-runtime --no-system-d3d-compiler --no-opengl --no-opengl-sw --no-angle

    CONFIG(debug, debug|release) : WINDEPLOY_FLAGS += --debug
    CONFIG(release, debug|release) : WINDEPLOY_FLAGS += --release

    # Windows specific libs
    #isEmpty(WINDOWS_SDK_LIB_PATH) {
    #   !contains(QMAKE_TARGET.arch, x86_64) {
    #       WINDOWS_SDK_LIB_PATH = "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17763.0\um\x86"
    #   } else {
    #       WINDOWS_SDK_LIB_PATH = "C:\Program Files (x86)\Windows Kits\10\Lib\10.0.17763.0\um\x86"
    #   }

    DEFINES += _WINSOCKAPI_
    #LIBS += -L$$WINDOWS_SDK_LIB_PATH
    LIBS += -lWs2_32
    LIBS += -liphlpapi
    LIBS += -luser32
    LIBS += -lole32

    # Find Simconnect location
    isEmpty(SIMCONNECT_PATH) {
        !contains(QMAKE_TARGET.arch, x86_64) {
            SIMCONNECT_PATH= "D:\Program Files (x86)\Microsoft Games\Microsoft Flight Simulator X SDK\SDK\Core Utilities Kit\SimConnect SDK"
            INCLUDEPATH += $$SIMCONNECT_PATH"\inc"
            SIMCONNECT_LIB_PATH = $$SIMCONNECT_PATH"\lib"
        } else {
            SIMCONNECT_PATH= "D:\Program Files\Lockheed Martin\Prepar3D v4 SDK 4.1.7.22841"
            INCLUDEPATH += $$SIMCONNECT_PATH"\inc\SimConnect"
            SIMCONNECT_LIB_PATH = $$SIMCONNECT_PATH"\lib\SimConnect"
        }
    }
    DEFINES += SIMCONNECT_BUILD

    CONFIG(debug, debug|release) {
        !contains(QMAKE_TARGET.arch, x86_64) {
            LIBS += -L$$SIMCONNECT_LIB_PATH -lSimConnect
        } else {
            LIBS += -L$$SIMCONNECT_LIB_PATH -lSimConnectDebug
        }
    } else {
      LIBS += -L$$SIMCONNECT_LIB_PATH -lSimConnect
    }

    # Find Miniupnpc location
    isEmpty(MINIUPNPC_PATH) {
        MINIUPNPC_PATH=..\miniupnp\miniupnpc
        !contains(QMAKE_TARGET.arch, x86_64) {
            MINIUPNPC_LIB_PATH = $$MINIUPNPC_PATH"\build32"
        }  else {
            MINIUPNPC_LIB_PATH = $$MINIUPNPC_PATH"\build64"
        }
    }
    DEFINES += MINIUPNP_STATICLIB
    INCLUDEPATH += $$MINIUPNPC_PATH
    CONFIG(debug, debug|release) {
        LIBS += -L$$MINIUPNPC_LIB_PATH"\Debug"
    } else {
        LIBS += -L$$MINIUPNPC_LIB_PATH"\Release"
    }
    LIBS += -llibminiupnpc

    # Find OpenSSL location
    isEmpty(OPENSSL_PATH) {
        !contains(QMAKE_TARGET.arch, x86_64) {
            OPENSSL_PATH="E:\devel\Qt\Tools\OpenSSL\Win_x86"
        } else {
            OPENSSL_PATH="E:\devel\Qt\Tools\OpenSSL\Win_x64"
        }
    }
    OPENSSL_LIB_PATH=$$OPENSSL_PATH"\lib"
    OPENSSL_DLL_PATH=$$OPENSSL_PATH"\bin"
    INCLUDEPATH += $$OPENSSL_PATH"\include"
    LIBS += -L$$OPENSSL_LIB_PATH -llibssl -llibcrypto

    # Find Crypto++ location
    isEmpty(CRYPTOPP_PATH) {
        CRYPTOPP_PATH=..\cryptopp
        !contains(QMAKE_TARGET.arch, x86_64) {
            CRYPTOPP_LIB_PATH=$$CRYPTOPP_PATH"\Win32\Output"
        } else {
            CRYPTOPP_LIB_PATH=$$CRYPTOPP_PATH"\x64\Output"
        }
    }
    INCLUDEPATH += $$CRYPTOPP_PATH
    CONFIG(release, debug|release) {
        LIBS += -L$$CRYPTOPP_LIB_PATH"\Release"
    } else {
        LIBS += -L$$CRYPTOPP_LIB_PATH"\Debug"
    }
    LIBS += -lcryptlib

    # Find ProtoBuf ressource
    isEmpty(PROTOBUF_PATH) {
        !contains(QMAKE_TARGET.arch, x86_64) {
            PROTOBUF_PATH="D:\Program Files (x86)\protobuf"
        } else {
            PROTOBUF_PATH="D:\Program Files\protobuf"
        }
    }
    PROTOBUF_LIB_PATH=$$PROTOBUF_PATH"\lib"
    INCLUDEPATH += $$PROTOBUF_PATH"\include"
    LIBS += -L$$PROTOBUF_LIB_PATH
    CONFIG(release, debug|release ) {
        LIBS += -llibprotobuf
    } else {
        LIBS += -llibprotobufd
    }
}

macx {

    # Mac Specific
    # Compatibility down to OS X 10.10
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.10
    QT_CONFIG -= no-pkg-config
    CONFIG += link_pkgconfig
    PKG_CONFIG = /usr/local/bin/pkg-config

    #add /usr/local scope which is not included by default on osx
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib
    LIBS += -framework CoreFoundation

    isEmpty(GIT_PATH) : GIT_PATH=git

    # Find Miniupnpc location
    isEmpty(MINIUPNPC_PATH) {
        MINIUPNPC_PATH=/usr/local/include
    }
    INCLUDEPATH += $$MINIUPNPC_PATH/miniupnpc
    LIBS += -lminiupnpc


    # Find OpenSSL location
    LIBS += -lssl

    # Find Crypto++ location
    isEmpty(CRYPTOPP_PATH){
        CRYPTOPP_PATH=/usr/local/include
    }
    INCLUDEPATH += $$CRYPTOPP_PATH/cryptopp
    LIBS += -lcrypto -lcryptopp

    # Find ProtoBuf ressource
    PROTOC = /usr/local/bin/protoc
    PKGCONFIG += protobuf
}

isEmpty(GIT_PATH) {
  GIT_REVISION='\\"UNKNOWN\\"'
} else {
  GIT_REVISION='\\"$$system('$$GIT_PATH' rev-parse --short HEAD)\\"'
}

#Protobuf generation

PROTO_DIR = $$PWD/protobuf
PROTOS = $$files(protobuf/*.proto)
include(protobuf.pri)

# Application Version
VERSION_MAJOR = 2
VERSION_MINOR = 1
VERSION_BUILD = 16
DEFINES += APP_NAME=\\\"$${TARGET}\\\"
DEFINES += APP_MAJOR=$$VERSION_MAJOR
DEFINES += APP_MINOR=$$VERSION_MINOR
DEFINES += APP_BUILD=$$VERSION_BUILD

QMAKE_EXTRA_TARGETS += versionTarget

DEFINES += GIT_REVISION=$$GIT_REVISION


# =======================================================================
# Print values when running qmake
!isEqual(QUIET, "true") {
message(-----------------------------------)
message(GIT_PATH: $$GIT_PATH)
message(GIT_REVISION: $$GIT_REVISION)
message(OPENSSL_PATH: $$OPENSSL_PATH)
message(MINIUPNPC_PATH: $$MINIUPNPC_PATH)

win32 : message(SIMCONNECT_PATH: $$SIMCONNECT_PATH)
message(DEPLOY_BASE: $$DEPLOY_BASE)
message(DEFINES: $$DEFINES)
message(INCLUDEPATH: $$INCLUDEPATH)
message(LIBS: $$LIBS)
message(TARGET_NAME: $$TARGET_NAME)
message(QT_INSTALL_PREFIX: $$[QT_INSTALL_PREFIX])
message(QT_INSTALL_LIBS: $$[QT_INSTALL_LIBS])
message(QT_INSTALL_PLUGINS: $$[QT_INSTALL_PLUGINS])
message(QT_INSTALL_TRANSLATIONS: $$[QT_INSTALL_TRANSLATIONS])
message(QT_INSTALL_BINS: $$[QT_INSTALL_BINS])
message(CONFIG: $$CONFIG)
message(-----------------------------------)
}

# =====================================================================
# Files

SOURCES += \
    main.cpp \
    disk/aibaseparser.cpp \
    disk/aimapping.cpp \
    disk/xpaiparser.cpp \
    disk/xpcslparser.cpp \
    gui/aireplace.cpp \
    gui/dialogp2p.cpp \
    gui/dialogprofils.cpp \
    gui/mainwindow.cpp \
    gui/mainwindow.GuiEvents.cpp \
    net/p2pmanager.cpp \
    net/peer.cpp \
    net/peer.Requests.cpp \
    net/peer.Event.cpp \
    net/udpserver.cpp \
    sim/aircraftstate.cpp \
    sim/analyzer.cpp \
    sim/fgdatagram.cpp \
    sim/fgmanager.cpp \
    sim/simbasemanager.cpp \
    sim/simconnector.cpp \
    sim/xpmanager.cpp \
    tools/calculation.cpp \
    tools/clogger.cpp \
    tools/crypto.cpp \
    tools/datagram.cpp \
    tools/datetime.cpp \
    tools/getappversion.cpp \
    tools/iniparser.cpp \
    tools/simple_file_parser.cpp \
    web/chttprequest.cpp \
    web/webmanager.cpp \
    web/webmanager.Callback.cpp \
    web/webmanager.Requests.cpp \
    web/webmanager.Utils.cpp \
    net/tcpserver.cpp \
    net/tcpclient.cpp

win32 {
SOURCES += \
    disk/scaiparser.cpp \
    sim/scmanager.cpp
}

macx {
SOURCES += \
    tools/keepalive.mm
}

HEADERS  +=  \
    disk/aibaseparser.h \
    disk/aimapping.h \
    disk/xpaiparser.h \
    disk/xpcslparser.h \
    gui/aireplace.h \
    gui/dialogp2p.h \
    gui/dialogprofils.h \
    gui/mainwindow.h \
    net/p2pmanager.h \
    net/peer.h \
    net/udpserver.h \
    sim/aircraftstate.h \
    sim/analyzer.h \
    sim/fgdatagram.h \
    sim/fgmanager.h \
    sim/simbasemanager.h \
    sim/simconnector.h \
    sim/xpmanager.h \
    tools/calculation.h \
    tools/call_once.h \
    tools/clogger.h \
    tools/crypto.h \
    tools/datagram.h \
    tools/datetime.h \
    tools/getappversion.h \
    tools/iniparser.h \
    tools/simple_file_parser.h \
    tools/singleton.h \
    tools/version.h \
    web/chttprequest.h \
    web/webmanager.h \
    net/tcpserver.h \
    net/tcpclient.h

win32 {
HEADERS += \
    disk/scaiparser.h \
    sim/scmanager.h
}

macx {
HEADERS += \
    tools/keepalive.h
}

FORMS += \
    gui/aireplace.ui \
    gui/dialogp2p.ui \
    gui/dialogprofils.ui \
    gui/mainwindow.ui


RESOURCES += \
    ressources.qrc

ICON = ffs2play.icns

win32 : RC_FILE = ffs2play.rc

OTHER_FILES += \
    README.md \
    ffs2play.iss \
    style/ffs2play.qss \
    changelog.txt \
    build-deb.sh

# =====================================================================
# Local deployment commands for development
# Linux - Copy help and Marble plugins and data
unix:!macx {
    copydata.commands += mkdir -p $$OUT_PWD/translations &&
    copydata.commands += cp -avfu $$PWD/intl/*.qm $$OUT_PWD/translations
}

# Mac OS X - Copy help and Marble plugins and data
macx {
    copydata.commands += cp -vf $$PWD/intl/*.qm $$OUT_PWD/ffs2play.app/Contents/Resources &&
    copydata.commands += cp -vf $$PWD/style/*.qss $$OUT_PWD/ffs2play.app/Contents/Resources
}

#Windows - Copy Qt Style and translation
win32 {
    defineReplace(p){return ($$shell_quote($$shell_path($$1)))}
    copydata.commands = xcopy /Y $$p($$PWD/intl/*.qm) $$p($$OUT_PWD/translations/) &&
    copydata.commands += xcopy /Y $$p($$PWD/style/*.qss) $$p($$OUT_PWD/)
}

# =====================================================================
# Deployment commands

#linux make install
unix:!macx {
    PREFIX = /usr
    Binary.path = $${PREFIX}/bin
    Binary.files = $$OUT_PWD/$${TARGET}
    INSTALLS += Binary

    Translation.path = $${PREFIX}/share/$${TARGET}/translations
    Translation.files = $$PWD/translations/*.qm
    INSTALLS += Translation

    Theme.path = $${PREFIX}/share/$${TARGET}
    Theme.files = $$PWD/style/*.qss
    INSTALLS += Theme

    Icons.path = $${PREFIX}/share/icons/hicolor/72x72/apps
    Icons.files = $$PWD/gfx/plane.png
    INSTALLS += Icons

    Shortcut.path = $${PREFIX}/share/applications
    Shortcut.files = $$PWD/*.desktop
    INSTALLS += Shortcut
}

# Linux specific deploy target
unix:!macx {
    DEPLOY_DIR=\"$$DEPLOY_BASE/$$TARGET_NAME\"
    DEPLOY_DIR_LIB=\"$$DEPLOY_BASE/$$TARGET_NAME/lib\"
    message(-----------------------------------)
    message(DEPLOY_DIR: $$DEPLOY_DIR)
    message(-----------------------------------)
    deploy.commands += rm -Rfv $$DEPLOY_DIR &&
    deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB &&
    deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB/iconengines &&
    deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB/imageformats &&
    deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB/platforms &&
    deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB/platformthemes &&
    deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB/printsupport &&
    deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB/sqldrivers &&
    deploy.commands += cp -Rvf $$OUT_PWD/translations $$DEPLOY_DIR &&
    deploy.commands += cp -vfa $$[QT_INSTALL_TRANSLATIONS]/qt_??.qm  $$DEPLOY_DIR/translations &&
    deploy.commands += cp -vfa $$[QT_INSTALL_TRANSLATIONS]/qt_??_??.qm  $$DEPLOY_DIR/translations &&
    deploy.commands += cp -vfa $$[QT_INSTALL_TRANSLATIONS]/qtbase*.qm  $$DEPLOY_DIR/translations &&
    deploy.commands += cp -vf $$PWD/desktop/qt.conf $$DEPLOY_DIR &&
    deploy.commands += cp -vf $$PWD/changelog.txt $$DEPLOY_DIR &&
    deploy.commands += cp -vf $$PWD/README.md $$DEPLOY_DIR &&
    deploy.commands += cp -vf $$PWD/LICENSE.GPLv3 $$DEPLOY_DIR &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/iconengines/libqsvgicon.so*  $$DEPLOY_DIR_LIB/iconengines &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/imageformats/libqgif.so*  $$DEPLOY_DIR_LIB/imageformats &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/imageformats/libqjpeg.so*  $$DEPLOY_DIR_LIB/imageformats &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/imageformats/libqsvg.so*  $$DEPLOY_DIR_LIB/imageformats &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/imageformats/libqwbmp.so*  $$DEPLOY_DIR_LIB/imageformats &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/imageformats/libqwebp.so*  $$DEPLOY_DIR_LIB/imageformats &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/platforms/libqeglfs.so*  $$DEPLOY_DIR_LIB/platforms &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/platforms/libqlinuxfb.so*  $$DEPLOY_DIR_LIB/platforms &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/platforms/libqminimal.so*  $$DEPLOY_DIR_LIB/platforms &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/platforms/libqminimalegl.so*  $$DEPLOY_DIR_LIB/platforms &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/platforms/libqoffscreen.so*  $$DEPLOY_DIR_LIB/platforms &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/platforms/libqxcb.so*  $$DEPLOY_DIR_LIB/platforms &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/platformthemes/libqgtk*.so*  $$DEPLOY_DIR_LIB/platformthemes &&
    deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlite.so*  $$DEPLOY_DIR_LIB/sqldrivers &&
    deploy.commands += cp -vfa $$OPENSSL_PATH/libssl.so.1.0.0 $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$OPENSSL_PATH/libcrypto.so.1.0.0 $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libicudata.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libicui18n.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libicuuc.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Concurrent.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Core.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5DBus.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Gui.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Network.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Qml.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Quick.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Script.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Sql.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Svg.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Widgets.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5X11Extras.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5XcbQpa.so*  $$DEPLOY_DIR_LIB &&
    deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Xml.so* $$DEPLOY_DIR_LIB
}

# Mac specific deploy target
macx {
    DEPLOY_DIR=\"$$PWD/../deploy\"
    message(-----------------------------------)
    message(DEPLOY_DIR: $$DEPLOY_DIR)
    message(-----------------------------------)
    deploy.commands += rm -Rfv $$DEPLOY_DIR/* &&
    !exists($$DEPLOY_DIR) : deploy.commands += mkdir -p $$DEPLOY_DIR &&
    deploy.commands += mkdir -p $$OUT_PWD/ffs2play.app/Contents/PlugIns &&
    deploy.commands += cp -fv $$[QT_INSTALL_TRANSLATIONS]/qt_??.qm  $$OUT_PWD/ffs2play.app/Contents/Resources &&
    deploy.commands += cp -fv $$[QT_INSTALL_TRANSLATIONS]/qt_??_??.qm  $$OUT_PWD/ffs2play.app/Contents/Resources &&
    deploy.commands += cp -fv $$[QT_INSTALL_TRANSLATIONS]/qtbase*.qm  $$OUT_PWD/ffs2play.app/Contents/Resources &&
    deploy.commands += macdeployqt ffs2play.app -always-overwrite -dmg &&
    deploy.commands += cp -fv $$PWD/LICENSE.GPLv3 $$DEPLOY_DIR &&
    deploy.commands += cp -fv $$PWD/README.md $$DEPLOY_DIR/README-FFS2Play.txt &&
    deploy.commands += cp -fv $$PWD/changelog.txt $$DEPLOY_DIR/CHANGELOG-FFS2Play.txt &&
    deploy.commands += cp -fv $$OUT_PWD/ffs2play.dmg $$DEPLOY_DIR/ffs2play_$${VERSION_MAJOR}_$${VERSION_MINOR}_$${VERSION_BUILD}_mac.dmg
}

# Windows specific deploy target
win32 {
    defineReplace(p){return ($$shell_quote($$shell_path($$1)))}
    RC_ICONS = ffs2play_48x48.ico
    CONFIG(debug, debug|release) : DLL_SUFFIX=d
    CONFIG(release, debug|release) : DLL_SUFFIX=
    deploy.commands = ( rmdir /s /q $$p($$DEPLOY_BASE) || echo Directory already empty) &&
    deploy.commands += mkdir $$p($$DEPLOY_BASE/$$TARGET_NAME/translations) &&
    deploy.commands += xcopy /Y $$p($$OUT_PWD/ffs2play.exe) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    deploy.commands += xcopy /Y $$p($$PWD/README.md) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    deploy.commands += xcopy /Y $$p($$PWD/intl/*.qm) $$p($$DEPLOY_BASE/$$TARGET_NAME/translations) &&
    deploy.commands += xcopy /Y $$p($$PWD/style/*.qss) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    deploy.commands += xcopy /Y $$p($$PWD/*.txt) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    deploy.commands += xcopy /Y $$p($$PWD/LICENSE.GPLv3) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    deploy.commands += xcopy /Y $$p($$PWD/*.iss) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    !contains(QMAKE_TARGET.arch, x86_64) {
        deploy.commands += xcopy /Y $$p($$SIMCONNECT_PATH/lib/SimConnect.msi) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
        deploy.commands += xcopy /Y $$p($$OPENSSL_DLL_PATH/libcrypto-1_1.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
        deploy.commands += xcopy /Y $$p($$OPENSSL_DLL_PATH/libssl-1_1.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    } else {
        deploy.commands += xcopy /Y $$p($$OPENSSL_DLL_PATH/libcrypto-1_1-x64.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
        deploy.commands += xcopy /Y $$p($$OPENSSL_DLL_PATH/libssl-1_1-x64.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    }
    deploy.commands += xcopy /Y $$p($$PWD/ffs2play_48x48.ico) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    deploy.commands += $$p($$[QT_INSTALL_BINS]/windeployqt) $$WINDEPLOY_FLAGS $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    !contains(QMAKE_TARGET.arch, x86_64) {
        deploy.commands += compil32 /cc $$p($$DEPLOY_BASE/$$TARGET_NAME/ffs2play.iss)
    } else {
        deploy.commands += compil32 /cc $$p($$DEPLOY_BASE/$$TARGET_NAME/ffs2play64.iss)
    }
}

# =====================================================================
# Additional targets

# Need to copy data when compiling
all.depends = copydata

# Deploy needs compiling before
deploy.depends = all

QMAKE_EXTRA_TARGETS += deploy copydata all
