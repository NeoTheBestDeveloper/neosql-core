#/bin/sh

meson setup -Db_sanitize=address,undefined --reconfigure build 
meson test neosql-core: -C build
