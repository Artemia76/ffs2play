FFS2PLAY

This project is a fork of the FFSTracker written in C# for FFS2 Flight Simulator virtual company.

FFS2PLAY will be designed on open source project model and will be writted in C++ with portable library (Qt)

The core will be universal to work with P2P multiplayer Layer

Linux and mac portage will be oriented X-PLANE

Windows will be compatible with P3D, FSX, FS SE and X-PLANE

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
be met: https://www.gnu.org/licenses/gpl-3.0.html.
1. DEPENDENCIES

- Last Qt Release (QT5.10 at this date)
 - Protobuf : 3.0.2 is advised, older can produce double free SIGABRT signal
 - miniupnpc : 1.9+
 - openssl : 1.0.2

2. BUILD
2.1 LINUX(UBUNTU)

 * On ubuntu you can enter this command line how provide you all dependencies needed to build ffs2play :

 sudo apt-get install build-essential libprotobuf-dev protobuf-compiler libssl-dev libminiupnpc-dev

 * on some qt installation you will need opengl devel :
	sudo apt-get install mesa-common-dev

2.2 WIN(Visual studio community edition)

coming ...
