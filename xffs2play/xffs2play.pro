#-------------------------------------------------
#
# Project created by QtCreator 2017-06-07T18:15:54
#
#-------------------------------------------------

QT += core
greaterThan(QT_MAJOR_VERSION, 4): QT += network xml

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
}

TEMPLATE = lib
CONFIG += warn_on shared
#CONFIG -= rtti exceptions thread qt
DEFINES += XFFS2PLAY_LIBRARY FFS_DEBUG

DEFINES += QT_DEPRECATED_WARNINGS

win32 {
    TARGET = win.xpl
    DEFINES += APL=0 IBM=1 LIN=0 WIN_32

    XPSDK_FOLDER = E:\devel\XPSDK213\SDK
    MINIUPNPC_PATH = "E:\devel\git\miniupnp\miniupnpc"
    PROTOBUF_LIB_PATH = "D:\Program Files (x86)\protobuf\lib"
    PROTOBUF_INCLUDE_PATH = "E:\devel\git\protobuf\src"
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
        $${MINIUPNPC_PATH} \
        $${OPENSSL_INCLUDE_PATH} \
        $${CRYPTOPP_INCLUDE_PATH} \
        $${PROTOBUF_INCLUDE_PATH}
    LIBS += -L$${XPSDK_FOLDER}/Libraries/Win
    LIBS += -lXPLM -lXPWidgets
}
unix {
    TARGET = lin.xpl
    DEFINES += APL=0 IBM=0 LIN=1
    QMAKE_CFLAGS += -fstack-protector
    QMAKE_CXXFLAGS += -fstack-protector
    XPSDK_FOLDER = /home/gianni/projet/XPSDK213/SDK
}
macx {
    TARGET = mac.xpl
    DEFINES += APL=1 IBM=0 LIN=0
    QMAKE_LFLAGS += -flat_namespace -undefined suppress
    CONFIG += x86 ppc
}

DEPENDPATH += ../ffs2core

INCLUDEPATH += $${XPSDK_FOLDER}/CHeaders/XPLM
#INCLUDEPATH += $${XPSDK_FOLDER}/CHeaders/Wrappers
INCLUDEPATH += $${XPSDK_FOLDER}/CHeaders/Widgets

INCLUDEPATH += ../ffs2core

LIBS	+= -L../ffs2core/$${DESTDIR} -lffs2core

SOURCES += \
        xffs2play.cpp \
        MainFrame.cpp

HEADERS += \
        xffs2play_global.h \ 
        MainFrame.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
