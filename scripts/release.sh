#!/bin/sh

meson setup --reconfigure --buildtype=release  build_release
meson compile -j 8 -C build_release
meson install -C build_release  --destdir .
