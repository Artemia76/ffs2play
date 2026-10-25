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
# Builds the FFS2Play AppImage : one file for every Linux distribution, Qt and OpenSSL included.
# Build it on an old distribution (Ubuntu 22.04 in the CI) : the AppImage runs on distributions
# with the same glibc or a newer one.
# The application knows it runs as an AppImage ($APPIMAGE) : its updates replace the file.
#
# Usage : packaging/linux/build-appimage.sh [options]
#   --qmake PATH       qmake of the Qt to bundle (default : $QT_ROOT_DIR/bin/qmake, else qmake6 / qmake)
#   --build-dir DIR    build directory (default : out/build/appimage)
#   --output DIR       where the AppImage is written (default : out/packages)
#   -h, --help         this help
#
# Tools : linuxdeploy and linuxdeploy-plugin-qt are downloaded in the build directory.

set -euo pipefail

SOURCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
QMAKE=""
BUILD_DIR="$SOURCE_DIR/out/build/appimage"
OUTPUT_DIR="$SOURCE_DIR/out/packages"
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous"
PLUGIN_QT_URL="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous"

die()  { echo "error : $*" >&2; exit 1; }
info() { echo "==> $*"; }

usage() { sed -n '/^# Usage/,/^$/s/^# \{0,1\}//p' "${BASH_SOURCE[0]}"; }

while [[ $# -gt 0 ]]; do
    case "$1" in
        --qmake)        QMAKE="${2:?--qmake needs a value}"; shift ;;
        --build-dir)    BUILD_DIR="${2:?--build-dir needs a value}"; shift ;;
        --output)       OUTPUT_DIR="${2:?--output needs a value}"; shift ;;
        -h|--help)      usage; exit 0 ;;
        *)              usage >&2; die "unknown option $1" ;;
    esac
    shift
done

if [[ -z "$QMAKE" ]]; then
    for _q in "${QT_ROOT_DIR:+$QT_ROOT_DIR/bin/qmake}" qmake6 qmake; do
        if [[ -n "$_q" ]] && command -v "$_q" >/dev/null 2>&1; then QMAKE="$(command -v "$_q")"; break; fi
    done
fi
[[ -n "$QMAKE" ]] || die "qmake not found : set QT_ROOT_DIR or use --qmake"
QT_PREFIX="$("$QMAKE" -query QT_INSTALL_PREFIX)"
info "Qt $("$QMAKE" -query QT_VERSION) ($QT_PREFIX)"

case "$(uname -m)" in
    x86_64)  ARCH=x86_64 ;;
    aarch64) ARCH=aarch64 ;;
    *)       die "unsupported architecture $(uname -m)" ;;
esac

mkdir -p "$BUILD_DIR" "$OUTPUT_DIR"
BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"
OUTPUT_DIR="$(cd "$OUTPUT_DIR" && pwd)"
APPDIR="$BUILD_DIR/AppDir"

# =======================================================================
# Build

info "Configuring in $BUILD_DIR"
cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_PREFIX_PATH="$QT_PREFIX" \
    -DFFS2PLAY_WITH_SIMCONNECT=OFF \
    -DFFS2PLAY_BUILD_TESTS=OFF \
    -DFFS2PLAY_PACKAGE_FORMAT=appimage

info "Building"
cmake --build "$BUILD_DIR"
VERSION="$(sed -n 's/^CMAKE_PROJECT_VERSION:STATIC=//p' "$BUILD_DIR/CMakeCache.txt")"

rm -rf "$APPDIR"
DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"

# =======================================================================
# Bundle

for _tool in "linuxdeploy-$ARCH.AppImage:$LINUXDEPLOY_URL" "linuxdeploy-plugin-qt-$ARCH.AppImage:$PLUGIN_QT_URL"; do
    _name="${_tool%%:*}"
    if [[ ! -x "$BUILD_DIR/$_name" ]]; then
        info "Downloading $_name"
        curl -fsSL -o "$BUILD_DIR/$_name" "${_tool#*:}/$_name"
        chmod +x "$BUILD_DIR/$_name"
    fi
done

# OpenSSL : Qt loads libssl at run time, it must match the bundled libcrypto
_ssl_libs=()
for _lib in libssl.so.3 libcrypto.so.3; do
    _path="$(ldconfig -p | awk -v lib="$_lib" '$1 == lib && /x86-64|AArch64/ { print $NF; exit }')"
    [[ -n "$_path" ]] || die "$_lib not found"
    _ssl_libs+=(--library "$_path")
done

info "Bundling Qt in the AppImage"
# containers have no FUSE : run the tools extracted
export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE
export LDAI_OUTPUT="$OUTPUT_DIR/ffs2play-$VERSION-$ARCH.AppImage"
# Wayland support when the Qt build provides it
if [[ -d "$("$QMAKE" -query QT_INSTALL_PLUGINS)/wayland-shell-integration" ]]; then
    export EXTRA_PLATFORM_PLUGINS="libqwayland-generic.so;libqwayland-egl.so"
fi
rm -f "$LDAI_OUTPUT"
(cd "$BUILD_DIR" && "./linuxdeploy-$ARCH.AppImage" --appdir "$APPDIR" \
    --desktop-file "$APPDIR/usr/share/applications/FFS2Play.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/72x72/apps/ffs2play.png" \
    "${_ssl_libs[@]}" \
    --plugin qt --output appimage)

[[ -f "$LDAI_OUTPUT" ]] || die "AppImage not produced"
info "AppImage : $LDAI_OUTPUT"
