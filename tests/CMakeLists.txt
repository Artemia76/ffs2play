find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(tst_core
    tst_core.cpp
    ${PROJECT_SOURCE_DIR}/tools/clogger.cpp ${PROJECT_SOURCE_DIR}/tools/clogger.h
    ${PROJECT_SOURCE_DIR}/tools/crypto.cpp ${PROJECT_SOURCE_DIR}/tools/crypto.h
    ${PROJECT_SOURCE_DIR}/tools/datagram.cpp ${PROJECT_SOURCE_DIR}/tools/datagram.h
    ${PROJECT_SOURCE_DIR}/net/upnpclient.cpp ${PROJECT_SOURCE_DIR}/net/upnpclient.h
)

target_include_directories(tst_core PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(tst_core PRIVATE
    Qt6::Core Qt6::Gui Qt6::Network Qt6::Xml Qt6::Protobuf Qt6::Test
    ffs2play_proto
    OpenSSL::Crypto)
if(WIN32)
    target_link_libraries(tst_core PRIVATE ws2_32 crypt32 advapi32 user32)
endif()

add_test(NAME tst_core COMMAND tst_core)
if(WIN32)
    # Find Qt and OpenSSL DLLs when running from the build tree
    get_filename_component(_ossl_root "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
    set_tests_properties(tst_core PROPERTIES
        ENVIRONMENT_MODIFICATION
        "PATH=path_list_prepend:${QT6_INSTALL_PREFIX}/bin;PATH=path_list_prepend:${_ossl_root}/bin")
endif()
