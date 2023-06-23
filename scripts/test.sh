#/bin/sh

meson setup -Db_sanitize=address,undefined --reconfigure build 
meson compile -j 8 -C build
meson test neosql-core: -C build
