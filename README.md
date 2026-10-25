#FFS2Play client build instruction

This project is a fork of the FFSTracker written in C# for FFS2 Flight Simulator virtual company.
FFS2PLAY will be designed on open source project model and will be writted in C++ with portable library (Qt)

The core will be universal to work with P2P multiplayer Layer

Linux and mac portage will be oriented X-PLANE and FlightGear

Windows will be compatible with P3D, FSX, FS SE, X-PLANE and FlightGear

FFS2Play is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

FFS2Play is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

The license is as published by the Free Software
Foundation and appearing in the file LICENSE.GPL3
included in the packaging of this software. Please review the following
information to ensure the GNU General Public License requirements will
be met: [GPLv3 licence ](https://www.gnu.org/licenses/gpl-3.0.html)

1. DEPENDENCIES

	- Qt : >= 6.8 with modules Core, Gui, Widgets, Network, Sql, Xml, Protobuf and Linguist tools,
	  and the private headers of Qt Gui / Qt Core (zip reader used to install the xffs2play plugin)
	- protoc (protobuf compiler) : >= 3.12, build time only (used by Qt qtprotobufgen)
	- OpenSSL : >= 3.0 (libcrypto only)
	- CMake : >= 3.27
	- Windows only, optional : MSFS / P3D / FSX SimConnect SDK (FSX, P3D and MSFS support)

	UPnP port mapping is built in (Qt Network), miniupnpc, Crypto++ and libprotobuf are no longer needed.

1. Build on Linux
	1. Native package for the running distribution (.deb, .rpm or Arch package), built against the distribution Qt
	   (Qt >= 6.8 : Debian 13, Ubuntu 25.04, Fedora 41, openSUSE Tumbleweed, Arch Linux or newer) :

			packaging/linux/build-package.sh --install-deps

		The package is written to out/packages/ ; `--help` lists the options (forced format, build folder, no tests).

	1. AppImage for every distribution (Qt from the Qt online installer or aqtinstall, on an old distribution
	   such as Ubuntu 22.04 for a wide glibc compatibility) :

			QT_ROOT_DIR=/path/to/Qt/6.8.3/gcc_64 packaging/linux/build-appimage.sh

	1. Debian (13 or newer) / Ubuntu (25.04 or newer)

			sudo apt-get install build-essential cmake ninja-build libssl-dev protobuf-compiler qt6-base-dev qt6-base-private-dev qt6-tools-dev qt6-l10n-tools libqt6sql6-sqlite qt6-grpc-dev qt6-grpc-tools

	1. Arch Linux

			sudo pacman -S base-devel cmake ninja openssl protobuf qt6-base qt6-tools qt6-grpc

	1. Final build:

		on git root :

			cmake --preset linux-release
			cmake --build --preset linux-release
			./out/build/linux-release/ffs2play

	1. Debian package:

			cpack --preset linux-release

1. Build on macOS

			brew install cmake ninja openssl@3 protobuf qt
			cmake --preset macos-release -DCMAKE_PREFIX_PATH="$(brew --prefix qt);$(brew --prefix openssl@3)"
			cmake --build --preset macos-release
			cpack --preset macos-release

1. Build on Windows (Visual Studio 2022 or newer)

	1. Install Qt >= 6.8 for MSVC 64 bit with the "Qt Protobuf and Qt GRPC" add-on and the OpenSSL 3 tool (Qt maintenance tool)
	1. Install protoc (https://github.com/protocolbuffers/protobuf/releases) and add it to PATH, or set Protobuf_ROOT
	1. Optional : set SIMCONNECT_PATH to the "SimConnect SDK" folder (MSFS_SDK environment variable is also used)
	1. Optional : install Inno Setup 6 or 7 to build the installer (cpack)

			set QTDIR=C:\Qt\6.8.3\msvc2022_64
			cmake --preset windows-msvc
			cmake --build --preset windows-msvc-release
			ctest --preset windows-msvc-release
			cpack --preset windows-msvc-release

	Local paths can be kept in a CMakeUserPresets.json (not versioned) inheriting the "windows-msvc" preset.
