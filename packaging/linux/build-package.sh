#!/usr/bin/env bash
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
#****************************************************************************
#
# Builds a native FFS2Play package for the Linux distribution it runs on,
# against the distribution's own Qt 6 (>= 6.8) :
#   Debian / Ubuntu / Mint ...        -> .deb  (CPack DEB, dependencies from dpkg-shlibdeps)
#   Fedora / RHEL / openSUSE ...      -> .rpm  (CPack RPM, dependencies from rpm autoreq)
#   Arch / Manjaro / EndeavourOS ...  -> .pkg.tar.zst (makepkg)
#   anything else                     -> .tar.gz (CPack TGZ, no dependency information)
#
# Usage : packaging/linux/build-package.sh [options]
#   --install-deps     install the build dependencies with the distribution package manager (sudo)
#   --format FMT       force the package format : deb, rpm, arch or tgz
#   --build-dir DIR    build directory (default : out/build/linux-package-<distribution>)
#   --output DIR       where the package is copied (default : out/packages)
#   --no-tests         do not run the unit tests
#   -h, --help         this help

set -euo pipefail

QT_MIN_VERSION=6.8

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
INSTALL_DEPS=0
RUN_TESTS=1
FORMAT=""
BUILD_DIR=""
OUTPUT_DIR="$SOURCE_DIR/out/packages"

die()  { echo "error : $*" >&2; exit 1; }
info() { echo "==> $*"; }

usage() { sed -n '/^# Usage/,/^$/s/^# \{0,1\}//p' "${BASH_SOURCE[0]}"; }

while [[ $# -gt 0 ]]; do
    case "$1" in
        --install-deps) INSTALL_DEPS=1 ;;
        --format)       FORMAT="${2:?--format needs a value}"; shift ;;
        --build-dir)    BUILD_DIR="${2:?--build-dir needs a value}"; shift ;;
        --output)       OUTPUT_DIR="${2:?--output needs a value}"; shift ;;
        --no-tests)     RUN_TESTS=0 ;;
        -h|--help)      usage; exit 0 ;;
        *)              usage >&2; die "unknown option $1" ;;
    esac
    shift
done

# =======================================================================
# Distribution detection

[[ -r /etc/os-release ]] || die "/etc/os-release not found : unknown distribution"
# shellcheck disable=SC1091
. /etc/os-release
DISTRO_ID="${ID:-linux}"
DISTRO_VERSION="${VERSION_ID:-}"
DISTRO_FAMILY=""
for _id in $DISTRO_ID ${ID_LIKE:-}; do
    case "$_id" in
        debian|ubuntu)                          DISTRO_FAMILY=debian ;;
        fedora|rhel|centos)                     DISTRO_FAMILY=fedora ;;
        suse|opensuse|opensuse-*|sles)          DISTRO_FAMILY=suse ;;
        arch|archlinux)                         DISTRO_FAMILY=arch ;;
    esac
    [[ -n "$DISTRO_FAMILY" ]] && break
done
info "Distribution : ${PRETTY_NAME:-$DISTRO_ID} (family : ${DISTRO_FAMILY:-unknown})"

if [[ -z "$FORMAT" ]]; then
    case "$DISTRO_FAMILY" in
        debian)      FORMAT=deb ;;
        fedora|suse) FORMAT=rpm ;;
        arch)        FORMAT=arch ;;
        *)           FORMAT=tgz
                     echo "warning : unsupported distribution, building a plain .tar.gz" >&2 ;;
    esac
fi
case "$FORMAT" in deb|rpm|arch|tgz) ;; *) die "unknown format $FORMAT (deb, rpm, arch or tgz)" ;; esac

# Package release tag and runtime dependencies the linker can not see (Qt plugins)
case "$DISTRO_FAMILY" in
    debian)
        PKG_RELEASE="1+${DISTRO_ID}${DISTRO_VERSION}"
        PKG_DEPENDS="libqt6sql6-sqlite, qt6-qpa-plugins" ;;
    fedora)
        if [[ "$DISTRO_ID" == fedora ]]; then
            PKG_RELEASE="1.fc${DISTRO_VERSION}"
        else
            PKG_RELEASE="1.el${DISTRO_VERSION%%.*}"
        fi
        PKG_DEPENDS="" ;;
    suse)
        PKG_RELEASE="1.${DISTRO_ID//-/_}${DISTRO_VERSION:+.${DISTRO_VERSION}}"
        PKG_DEPENDS="qt6-sql-sqlite" ;;
    *)
        PKG_RELEASE="1"
        PKG_DEPENDS="" ;;
esac
# '-' is the version / release separator in both deb and rpm
PKG_RELEASE="${PKG_RELEASE//-/_}"

# =======================================================================
# Build dependencies

SUDO=""
[[ $EUID -ne 0 ]] && SUDO="sudo"

# The package links against the distribution Qt : it must be recent enough
check_qt_version() {
    if [[ "$(printf '%s\n%s\n' "$QT_MIN_VERSION" "$1" | sort -V | head -n1)" != "$QT_MIN_VERSION" ]]; then
        die "the distribution ships Qt $1, FFS2Play needs Qt >= $QT_MIN_VERSION (e.g. Debian 13, Ubuntu 25.04, Fedora 41 or newer)"
    fi
}

install_deps() {
    case "$DISTRO_FAMILY" in
        debian)
            $SUDO apt-get update
            # Qt Protobuf only exists from Qt 6.8 (Debian 13, Ubuntu 25.04) : say so before apt fails
            local _qt_candidate
            _qt_candidate="$(apt-cache policy qt6-base-dev | sed -n 's/^ *Candidate: *\([0-9.]*\).*/\1/p')"
            check_qt_version "${_qt_candidate:-0}"
            $SUDO apt-get install -y --no-install-recommends \
                build-essential cmake ninja-build git pkg-config dpkg-dev file \
                libssl-dev protobuf-compiler libgl-dev libxkbcommon-dev \
                qt6-base-dev qt6-base-dev-tools qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools \
                qt6-grpc-dev qt6-grpc-tools libqt6sql6-sqlite qt6-qpa-plugins ;;
        fedora)
            $SUDO dnf install -y \
                gcc-c++ cmake ninja-build git rpm-build openssl-devel protobuf-compiler \
                qt6-qtbase-devel qt6-qttools-devel qt6-linguist qt6-qtgrpc-devel ;;
        suse)
            $SUDO zypper --non-interactive install \
                gcc-c++ cmake ninja git rpm-build libopenssl-devel protobuf-devel \
                qt6-base-devel qt6-tools-devel qt6-linguist-devel qt6-protobuf-devel \
                qt6-sql-sqlite ;;
        arch)
            $SUDO pacman -S --needed --noconfirm \
                base-devel cmake ninja git openssl protobuf qt6-base qt6-tools qt6-grpc ;;
        *)
            die "--install-deps : unsupported distribution, install the dependencies listed in README.md" ;;
    esac
}

if [[ $INSTALL_DEPS -eq 1 ]]; then
    info "Installing build dependencies"
    install_deps
fi

for _tool in cmake ninja protoc; do
    command -v "$_tool" >/dev/null || die "$_tool not found (run with --install-deps)"
done

QMAKE=""
for _q in qmake6 qmake-qt6 /usr/lib/qt6/bin/qmake /usr/lib64/qt6/bin/qmake \
          "/usr/lib/$(uname -m)-linux-gnu/qt6/bin/qmake"; do
    if command -v "$_q" >/dev/null 2>&1; then QMAKE="$_q"; break; fi
done
[[ -n "$QMAKE" ]] || die "Qt 6 not found (run with --install-deps)"
QT_VERSION="$("$QMAKE" -query QT_VERSION)"
check_qt_version "$QT_VERSION"
info "Qt $QT_VERSION ($QMAKE)"

[[ -d "$SOURCE_DIR/.git" ]] || echo "warning : no git repository, the version will be 0.0.1" >&2

# =======================================================================
# Build

[[ -n "$BUILD_DIR" ]] || BUILD_DIR="$SOURCE_DIR/out/build/linux-package-${DISTRO_ID}"
mkdir -p "$BUILD_DIR" "$OUTPUT_DIR"
BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"
OUTPUT_DIR="$(cd "$OUTPUT_DIR" && pwd)"

info "Configuring in $BUILD_DIR"
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DFFS2PLAY_WITH_SIMCONNECT=OFF \
    -DFFS2PLAY_BUILD_TESTS=$([[ $RUN_TESTS -eq 1 ]] && echo ON || echo OFF) \
    -DFFS2PLAY_PACKAGE_RELEASE="$PKG_RELEASE" \
    -DFFS2PLAY_PACKAGE_DEPENDS="$PKG_DEPENDS"

info "Building"
cmake --build "$BUILD_DIR"

if [[ $RUN_TESTS -eq 1 ]]; then
    info "Running unit tests"
    QT_QPA_PLATFORM=offscreen ctest --test-dir "$BUILD_DIR" --output-on-failure
fi

VERSION="$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$BUILD_DIR/CMakeCache.txt")"

# =======================================================================
# Package

package_arch() {
    # makepkg packages the build tree installed by cmake (nothing is rebuilt)
    command -v makepkg >/dev/null || die "makepkg not found"
    [[ $EUID -ne 0 ]] || die "makepkg can not run as root : run this script as a normal user"
    local _pkgdir="$BUILD_DIR/makepkg"
    rm -rf "$_pkgdir"
    mkdir -p "$_pkgdir"
    cat > "$_pkgdir/PKGBUILD" <<EOF
pkgname=ffs2play
pkgver=$VERSION
pkgrel=1
pkgdesc="FFS2Play peer to peer multiplayer client for flight simulators"
arch=('$(uname -m)')
url="http://ffs2play.fr/"
license=('GPL-3.0-or-later')
depends=('qt6-base' 'qt6-grpc' 'openssl' 'hicolor-icon-theme')
options=('!debug')

package() {
    DESTDIR="\$pkgdir" cmake --install "$BUILD_DIR"
}
EOF
    (cd "$_pkgdir" && PKGDEST="$OUTPUT_DIR" makepkg --force --nodeps) >&2
    ls "$OUTPUT_DIR"/ffs2play-"$VERSION"-1-*.pkg.tar.* 2>/dev/null | grep -v '\.sig$' | tail -n1
}

package_cpack() {
    local _gen="$1" _pattern="$2"
    [[ "$_gen" == DEB ]] && { command -v dpkg-shlibdeps >/dev/null || die "dpkg-shlibdeps not found (package dpkg-dev)"; }
    [[ "$_gen" == RPM ]] && { command -v rpmbuild >/dev/null || die "rpmbuild not found (package rpm-build)"; }
    rm -f "$BUILD_DIR"/$_pattern
    (cd "$BUILD_DIR" && cpack -G "$_gen") >&2
    local _file
    _file="$(ls "$BUILD_DIR"/$_pattern | tail -n1)"
    cp "$_file" "$OUTPUT_DIR/"
    echo "$OUTPUT_DIR/$(basename "$_file")"
}

info "Packaging ($FORMAT)"
case "$FORMAT" in
    deb)  PACKAGE="$(package_cpack DEB 'ffs2play_*.deb')" ;;
    rpm)  PACKAGE="$(package_cpack RPM 'ffs2play-*.rpm')" ;;
    tgz)  PACKAGE="$(package_cpack TGZ 'ffs2play_*.tar.gz')" ;;
    arch) PACKAGE="$(package_arch)" ;;
esac

info "Package : $PACKAGE"
case "$FORMAT" in
    deb)  echo "    install : sudo apt install $PACKAGE" ;;
    rpm)  if [[ "$DISTRO_FAMILY" == suse ]]; then
              echo "    install : sudo zypper install $PACKAGE"
          else
              echo "    install : sudo dnf install $PACKAGE"
          fi ;;
    arch) echo "    install : sudo pacman -U $PACKAGE" ;;
esac
