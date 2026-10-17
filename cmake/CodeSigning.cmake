# Authenticode signing of ffs2play.exe, of the installer and of its uninstaller.
#
# Signing is enabled when FFS2PLAY_SIGN_ARGS is set : it holds the signtool
# options selecting the certificate, for example
#   certificate in the Windows store  : /sha1 0123456789ABCDEF0123456789ABCDEF01234567
#   Certum SimplySign (cloud HSM)     : /n "Open Source Developer, Your Name"
#   Azure Artifact (Trusted) Signing  : /dlib "C:/.../Azure.CodeSigning.Dlib.dll" /dmdf "C:/.../metadata.json"
# Timestamping keeps the signature valid after the certificate expires.

set(FFS2PLAY_SIGN_ARGS "" CACHE STRING "signtool arguments selecting the code signing certificate (empty : no signing)")
set(FFS2PLAY_SIGN_TIMESTAMP_URL "http://timestamp.digicert.com" CACHE STRING "RFC 3161 timestamp server")

if(NOT WIN32 OR NOT FFS2PLAY_SIGN_ARGS)
    return()
endif()

set(_pf86 "ProgramFiles(x86)")
file(GLOB _sdk_signtools "$ENV{${_pf86}}/Windows Kits/10/bin/10.*/x64/signtool.exe")
list(SORT _sdk_signtools COMPARE NATURAL ORDER DESCENDING)
set(_sdk_signtool_dirs)
foreach(_tool IN LISTS _sdk_signtools)
    get_filename_component(_dir "${_tool}" DIRECTORY)
    list(APPEND _sdk_signtool_dirs "${_dir}")
endforeach()
find_program(FFS2PLAY_SIGNTOOL signtool HINTS ${_sdk_signtool_dirs})
if(NOT FFS2PLAY_SIGNTOOL)
    message(FATAL_ERROR "FFS2PLAY_SIGN_ARGS is set but signtool.exe was not found (install the Windows SDK)")
endif()

set(_sign_options /fd SHA256 /tr "${FFS2PLAY_SIGN_TIMESTAMP_URL}" /td SHA256)
separate_arguments(_cert_options WINDOWS_COMMAND "${FFS2PLAY_SIGN_ARGS}")
message(STATUS "Code signing enabled with ${FFS2PLAY_SIGNTOOL}")

# Sign the installed executable (also runs when CPack stages the package)
install(CODE "
    set(_exe \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/ffs2play.exe\")
    message(STATUS \"Signing: \${_exe}\")
    execute_process(
        COMMAND \"${FFS2PLAY_SIGNTOOL}\" sign ${_sign_options} ${_cert_options} \"\${_exe}\"
        RESULT_VARIABLE _sign_result)
    if(NOT _sign_result EQUAL 0)
        message(FATAL_ERROR \"signtool failed on \${_exe}\")
    endif()
")

# Sign the generated installer once CPack has built it.
# (Inno Setup's own SignTool cannot be used : CPack does not quote the /S
# argument it passes to ISCC, so any path with spaces breaks it.)
set(FFS2PLAY_SIGN_COMMAND "${FFS2PLAY_SIGNTOOL}" sign ${_sign_options} ${_cert_options})
configure_file(${CMAKE_CURRENT_LIST_DIR}/SignPackage.cmake.in
               ${CMAKE_BINARY_DIR}/SignPackage.cmake @ONLY)
set(CPACK_POST_BUILD_SCRIPTS "${CMAKE_BINARY_DIR}/SignPackage.cmake")
