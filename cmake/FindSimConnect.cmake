# FindSimConnect
# -------------
# Locate the Microsoft SimConnect SDK (MSFS 2020/2024 SDK, or FSX / P3D SDK).
#
# Search hints, in order :
#   SIMCONNECT_PATH (CMake or environment variable) : root of the "SimConnect SDK" folder
#   MSFS_SDK / MSFS2024_SDK (environment, set by the MSFS SDK installer)
#
# Imported target :
#   SimConnect::SimConnect
#
# Result variables :
#   SimConnect_FOUND, SimConnect_INCLUDE_DIR, SimConnect_LIBRARY, SimConnect_DLL

set(_simconnect_hints)
foreach(_root IN ITEMS "${SIMCONNECT_PATH}" "$ENV{SIMCONNECT_PATH}")
    if(_root)
        list(APPEND _simconnect_hints "${_root}")
    endif()
endforeach()
foreach(_sdk IN ITEMS "$ENV{MSFS2024_SDK}" "$ENV{MSFS_SDK}")
    if(_sdk)
        list(APPEND _simconnect_hints "${_sdk}/SimConnect SDK")
    endif()
endforeach()
list(APPEND _simconnect_hints
    "C:/MSFS 2024 SDK/SimConnect SDK"
    "C:/MSFS SDK/SimConnect SDK"
    "D:/MSFS SDK/SimConnect SDK")

find_path(SimConnect_INCLUDE_DIR
    NAMES SimConnect.h
    HINTS ${_simconnect_hints}
    PATH_SUFFIXES include inc)

# Prefer the static library (no SimConnect.dll to ship)
find_library(SimConnect_LIBRARY_RELEASE
    NAMES SimConnect
    HINTS ${_simconnect_hints}
    PATH_SUFFIXES lib/static lib)
find_library(SimConnect_LIBRARY_DEBUG
    NAMES SimConnect_debug
    HINTS ${_simconnect_hints}
    PATH_SUFFIXES lib/static lib)
find_file(SimConnect_DLL
    NAMES SimConnect.dll
    HINTS ${_simconnect_hints}
    PATH_SUFFIXES lib)

include(SelectLibraryConfigurations)
select_library_configurations(SimConnect)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SimConnect
    REQUIRED_VARS SimConnect_LIBRARY SimConnect_INCLUDE_DIR)

if(SimConnect_FOUND AND NOT TARGET SimConnect::SimConnect)
    add_library(SimConnect::SimConnect UNKNOWN IMPORTED)
    set_target_properties(SimConnect::SimConnect PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SimConnect_INCLUDE_DIR}"
        IMPORTED_LOCATION "${SimConnect_LIBRARY_RELEASE}")
    if(SimConnect_LIBRARY_DEBUG)
        set_target_properties(SimConnect::SimConnect PROPERTIES
            IMPORTED_LOCATION_DEBUG "${SimConnect_LIBRARY_DEBUG}"
            IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    endif()
    # The static SimConnect library needs these system libraries
    set_property(TARGET SimConnect::SimConnect APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES shlwapi ws2_32 user32 ole32 advapi32)
endif()

mark_as_advanced(SimConnect_INCLUDE_DIR SimConnect_LIBRARY_RELEASE SimConnect_LIBRARY_DEBUG SimConnect_DLL)
