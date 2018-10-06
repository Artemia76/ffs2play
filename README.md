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

	- Qt : >=5.9
	- Protobuf : >=3.0.0
	- miniupnpc : >=1.9
	- openssl : >=1.0.2
	- crypto++ : >= 5.6

1. Build on Linux
	1. UBUNTU

		On ubuntu you can enter this command line how provide you all dependencies needed to build ffs2play:

			sudo apt-get install build-essential libssl-dev libminiupnpc-dev libcrypto++-dev libprotobuf-dev protobuf-compiler qtbase5-dev

		On some qt installation you will need opengl devel :

			sudo apt-get install mesa-common-dev

		On Xenial , Qt packages are too old for FFS2Play , you can use PPA to give you opportunity de build on supported packages:

			sudo add-apt-repository ppa:beineri/opt-qt-5.10.1-xenial
			sudo apt-get update
			sudo apt-get install qt510-meta-full qt510creator

	1. Gentoo / Calculate Linux

		Prepare dependencies on terminal as superuser:

			emerge crypto++ miniupnpc openssl protobuf @qt5-essentials

	1. Arch Linux

		Prepare dependencies on terminal :

			sudo yaourt -S base-devel gdb qt5-base miniupnpc crypto++ openssl protobuf

	1. Final build:

		on git root :

			qmake ffs2play.pro
			make -j3
			./release/ffs2play

1. WIN(Visual studio community edition)

	coming ...
