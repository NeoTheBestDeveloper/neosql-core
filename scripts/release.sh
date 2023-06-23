#!/bin/sh

meson setup --reconfigure --buildtype=release  build_release
meson compile -j 8 -C build_release

meson setup --cross-file x86_64-w64-mingw32.txt --buildtype=release --reconfigure build-mingw_release
meson compile -j 8 -C build-mingw_release

meson install -C build_release  --destdir .
meson install -C build-mingw_release --destdir .
