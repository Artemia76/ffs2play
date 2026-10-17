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

cmake_minimum_required(VERSION 3.27)

# =======================================================================
# Application version from the last git tag (x.y.z)

set(FFS2PLAY_VERSION "0.0.1")
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --abbrev=0 --tags
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE _git_tag
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET)
    if(_git_tag MATCHES "^v?([0-9]+\\.[0-9]+\\.[0-9]+)")
        set(FFS2PLAY_VERSION "${CMAKE_MATCH_1}")
    endif()
endif()

project(ffs2play
    VERSION ${FFS2PLAY_VERSION}
    DESCRIPTION "FFS2Play peer to peer multiplayer client for flight simulators"
    HOMEPAGE_URL "http://ffs2play.fr/"
    LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

# =======================================================================
# Options

# The SimConnect SDK only ships MSVC libraries
if(WIN32 AND MSVC)
    set(_simconnect_default ON)
else()
    set(_simconnect_default OFF)
endif()
option(FFS2PLAY_WITH_SIMCONNECT "Build FSX / P3D / MSFS support through SimConnect (Windows / MSVC only)" ${_simconnect_default})
option(FFS2PLAY_BUILD_TESTS "Build unit tests" ON)

# =======================================================================
# Dependencies

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets Network Sql Xml Protobuf LinguistTools)
qt_standard_project_setup(I18N_SOURCE_LANGUAGE en I18N_TRANSLATED_LANGUAGES fr_FR es_ES)
# qt_standard_project_setup only enables AUTOMOC / AUTOUIC : ressources.qrc needs AUTORCC
set(CMAKE_AUTORCC ON)

# Qt online installer ships OpenSSL 3 under Tools/OpenSSLv3
if(WIN32 AND NOT OPENSSL_ROOT_DIR AND NOT DEFINED ENV{OPENSSL_ROOT_DIR})
    cmake_path(SET _qt_openssl NORMALIZE "${QT6_INSTALL_PREFIX}/../../Tools/OpenSSLv3/Win_x64")
    if(EXISTS "${_qt_openssl}")
        set(OPENSSL_ROOT_DIR "${_qt_openssl}")
    endif()
endif()
find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)

if(FFS2PLAY_WITH_SIMCONNECT)
    find_package(SimConnect)
    if(NOT SimConnect_FOUND)
        message(WARNING "SimConnect SDK not found (set SIMCONNECT_PATH or MSFS_SDK) : "
                        "FSX / P3D / MSFS support disabled")
        set(FFS2PLAY_WITH_SIMCONNECT OFF)
    endif()
endif()

# =======================================================================
# Protocol buffers (Qt Protobuf)

qt_add_library(ffs2play_proto STATIC)
qt_add_protobuf(ffs2play_proto
    PROTO_FILES protobuf/ffs2play.proto)

# =======================================================================
# Application

qt_add_executable(ffs2play WIN32 MACOSX_BUNDLE
    main.cpp
    disk/aibaseparser.cpp disk/aibaseparser.h
    disk/aimapping.cpp disk/aimapping.h
    disk/xpaiparser.cpp disk/xpaiparser.h
    disk/xpcslparser.cpp disk/xpcslparser.h
    gui/aireplace.cpp gui/aireplace.h gui/aireplace.ui
    gui/dialogp2p.cpp gui/dialogp2p.h gui/dialogp2p.ui
    gui/dialogprofils.cpp gui/dialogprofils.h gui/dialogprofils.ui
    gui/mainwindow.cpp gui/mainwindow.GuiEvents.cpp gui/mainwindow.h gui/mainwindow.ui
    net/p2pmanager.cpp net/p2pmanager.h
    net/peer.cpp net/peer.Requests.cpp net/peer.Event.cpp net/peer.h
    net/tcpclient.cpp net/tcpclient.h
    net/tcpserver.cpp net/tcpserver.h
    net/udpserver.cpp net/udpserver.h
    net/upnpclient.cpp net/upnpclient.h
    sim/aircraftstate.cpp sim/aircraftstate.h
    sim/analyzer.cpp sim/analyzer.h
    sim/fgdatagram.cpp sim/fgdatagram.h
    sim/fgmanager.cpp sim/fgmanager.h
    sim/simbasemanager.cpp sim/simbasemanager.h
    sim/simconnector.cpp sim/simconnector.h
    sim/xpmanager.cpp sim/xpmanager.h
    tools/calculation.cpp tools/calculation.h
    tools/clogger.cpp tools/clogger.h
    tools/crypto.cpp tools/crypto.h
    tools/datagram.cpp tools/datagram.h
    tools/datetime.cpp tools/datetime.h
    tools/getappversion.cpp tools/getappversion.h
    tools/iniparser.cpp tools/iniparser.h
    tools/simple_file_parser.cpp tools/simple_file_parser.h
    tools/singleton.h
    tools/version.h
    web/chttprequest.cpp web/chttprequest.h
    web/caddrrequest.cpp web/caddrrequest.h
    web/webmanager.cpp web/webmanager.Callback.cpp web/webmanager.Requests.cpp
    web/webmanager.Utils.cpp web/webmanager.h
    ressources.qrc
)

target_include_directories(ffs2play PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(ffs2play PRIVATE
    APP_NAME="${PROJECT_NAME}"
    APP_MAJOR=${PROJECT_VERSION_MAJOR}
    APP_MINOR=${PROJECT_VERSION_MINOR}
    APP_BUILD=${PROJECT_VERSION_PATCH}
    $<$<CONFIG:Debug>:FFS_DEBUG=1>
    QT_DISABLE_DEPRECATED_UP_TO=0x060800
)

target_link_libraries(ffs2play PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network Qt6::Sql Qt6::Xml Qt6::Protobuf
    ffs2play_proto
    OpenSSL::Crypto
)

if(WIN32)
    target_sources(ffs2play PRIVATE ffs2play.rc)
    target_compile_definitions(ffs2play PRIVATE _WINSOCKAPI_ NOMINMAX)
    target_link_libraries(ffs2play PRIVATE shlwapi ws2_32 iphlpapi user32 ole32)
    if(OPENSSL_USE_STATIC_LIBS)
        target_link_libraries(ffs2play PRIVATE crypt32 advapi32)
    endif()
endif()

if(FFS2PLAY_WITH_SIMCONNECT)
    target_sources(ffs2play PRIVATE
        disk/scaiparser.cpp disk/scaiparser.h
        sim/scmanager.cpp sim/scmanager.h)
    target_compile_definitions(ffs2play PRIVATE SIMCONNECT_BUILD)
    target_link_libraries(ffs2play PRIVATE SimConnect::SimConnect)
    # The SDK ships SimConnect_debug.lib without its .pdb
    target_link_options(ffs2play PRIVATE $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/IGNORE:4099>)
endif()

if(APPLE)
    target_sources(ffs2play PRIVATE tools/keepalive.mm tools/keepalive.h)
    target_link_libraries(ffs2play PRIVATE "-framework Foundation" "-framework CoreFoundation")
    set(MACOSX_BUNDLE_ICON_FILE ffs2play.icns)
    set(_app_icon "${CMAKE_CURRENT_SOURCE_DIR}/ffs2play.icns")
    set_source_files_properties(${_app_icon} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(ffs2play PRIVATE ${_app_icon})
    set_target_properties(ffs2play PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "FFS2Play"
        MACOSX_BUNDLE_GUI_IDENTIFIER "fr.ffsimulateur2.ffs2play"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
endif()

# Style sheet embedded in resources
qt_add_resources(ffs2play "style"
    PREFIX "/"
    FILES style/ffs2play.qss)

# Translations : .ts are compiled to .qm and embedded under :/i18n
qt_add_translations(ffs2play
    TS_FILES intl/ffs2play_fr_FR.ts intl/ffs2play_es_ES.ts
    RESOURCE_PREFIX "/i18n")

# =======================================================================
# Install & deployment

install(TARGETS ffs2play
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(WIN32)
    set(_doc_dir ${CMAKE_INSTALL_BINDIR})
elseif(APPLE)
    set(_doc_dir .)
else()
    set(_doc_dir ${CMAKE_INSTALL_DOCDIR})
endif()
install(FILES README.md changelog.txt LICENSE.GPLv3 DESTINATION ${_doc_dir})

if(UNIX AND NOT APPLE)
    install(FILES FFS2Play.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES gfx/plane.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/72x72/apps
        RENAME ffs2play.png)
endif()

if(WIN32 OR APPLE)
    set(_deploy_options)
    if(WIN32)
        # windeployqt only options (macdeployqt rejects them)
        set(_deploy_options DEPLOY_TOOL_OPTIONS --no-opengl-sw --no-system-d3d-compiler)
    endif()
    qt_generate_deploy_app_script(
        TARGET ffs2play
        OUTPUT_SCRIPT _deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
        ${_deploy_options})
    install(SCRIPT ${_deploy_script})
endif()

if(WIN32 AND MSVC)
    # Ship the Visual C++ runtime next to the executable
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
    include(InstallRequiredSystemLibraries)
endif()

if(WIN32 AND NOT OPENSSL_USE_STATIC_LIBS)
    # libcrypto DLL lives next to the import library's bin folder
    get_filename_component(_ossl_root "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
    file(GLOB _ossl_dlls "${_ossl_root}/bin/libcrypto*.dll")
    if(_ossl_dlls)
        install(FILES ${_ossl_dlls} DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endif()

# Authenticode signing (Windows, when FFS2PLAY_SIGN_ARGS is set)
include(CodeSigning)

# =======================================================================
# Packaging

set(CPACK_PACKAGE_NAME "FFS2Play")
set(CPACK_PACKAGE_VENDOR "FFS2")
set(CPACK_PACKAGE_CONTACT "admin@ffs2play.fr")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "FFS2Play")
set(CPACK_PACKAGE_EXECUTABLES "ffs2play" "FFS2Play")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.GPLv3")
set(CPACK_PACKAGE_FILE_NAME
    "ffs2play_${PROJECT_VERSION_MAJOR}_${PROJECT_VERSION_MINOR}_${PROJECT_VERSION_PATCH}_${CMAKE_SYSTEM_NAME}")

if(WIN32)
    set(CPACK_GENERATOR INNOSETUP)
    # CPack only looks for Inno Setup 6 by default
    set(_pf86 "ProgramFiles(x86)")
    find_program(FFS2PLAY_ISCC_EXECUTABLE ISCC
        HINTS "$ENV{ProgramFiles}/Inno Setup 7" "$ENV{${_pf86}}/Inno Setup 7"
              "$ENV{ProgramFiles}/Inno Setup 6" "$ENV{${_pf86}}/Inno Setup 6")
    if(FFS2PLAY_ISCC_EXECUTABLE)
        set(CPACK_INNOSETUP_EXECUTABLE "${FFS2PLAY_ISCC_EXECUTABLE}")
    endif()
    set(CPACK_INNOSETUP_ARCHITECTURE x64)
    set(CPACK_INNOSETUP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ffs2play_48x48.ico")
    set(CPACK_INNOSETUP_LANGUAGES english french spanish)
    set(CPACK_INNOSETUP_USE_MODERN_WIZARD ON)
    set(CPACK_INNOSETUP_RUN_EXECUTABLES ffs2play)
    set(CPACK_CREATE_DESKTOP_LINKS ffs2play)
    set(CPACK_INNOSETUP_SETUP_AppId "{{7E043FC6-3D67-46C1-BB94-AC9C9E278E3D}")
    set(CPACK_INNOSETUP_SETUP_InfoBeforeFile "${CMAKE_CURRENT_SOURCE_DIR}/changelog.txt")
    set(CPACK_INNOSETUP_EXTRA_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/innosetup_cleanup.iss")
elseif(APPLE)
    set(CPACK_GENERATOR DragNDrop)
else()
    set(CPACK_GENERATOR DEB)
    set(CPACK_DEBIAN_PACKAGE_SECTION games)
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
endif()

include(CPack)

# =======================================================================
# Tests

if(FFS2PLAY_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
