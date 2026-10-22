# FindSimConnect
# -------------
# Locate the Microsoft SimConnect SDK (MSFS 2020/2024 SDK, or FSX / P3D SDK).
#
# Search hints, in order :
#   SIMCONNECT_PATH (CMake or environment variable) : root of the "SimConnect SDK" folder
#   MSFS2024_SDK, C:/MSFS 2024 SDK, then MSFS_SDK (MSFS 2020 SDK installer)
# The MSFS 2024 SDK comes first : it also builds clients for MSFS 2020, and adds the
# aircraft enumeration (SIMCONNECT_MSFS2024 is defined by CMakeLists.txt when present).
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
if(DEFINED ENV{MSFS2024_SDK})
    list(APPEND _simconnect_hints "$ENV{MSFS2024_SDK}/SimConnect SDK")
endif()
list(APPEND _simconnect_hints "C:/MSFS 2024 SDK/SimConnect SDK")
if(DEFINED ENV{MSFS_SDK})
    list(APPEND _simconnect_hints "$ENV{MSFS_SDK}/SimConnect SDK")
endif()
list(APPEND _simconnect_hints
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
