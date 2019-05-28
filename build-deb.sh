#! /bin/bash
mkdir deb-build
cd deb-build
sudo checkinstall \
    --pkgsource="http://ffs2play.fr/" \
    --pkglicense="GPL3" \
    --pkggroup="games" \
    --deldesc=no \
    --nodoc \
    --maintainer="gianni" \
    --pkgarch=$(dpkg \
    --print-architecture) \
    --pkgversion="2.1" \
    --pkgrelease="8" \
    --pkgname=ffs2play \
    --require="libprotobuf10,libminiupnpc10,libcrypto++6,libqt5concurrent5,libqt5core5a,libqt5dbus5,libqt5gui5,libqt5network5,libqt5opengl5,libqt5printsupport5,libqt5sql5,libqt5sql5-sqlite,libqt5svg5,libqt5widgets5,libqt5xml5" \
    --install=no


