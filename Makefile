.PHONY: clean, test 

clean:
	rm -rf build
	rm -rf build_release
	rm -rf build-mingw
	rm -rf build-mingw_release
	rm -rf .cache

release:
	meson setup --wipe --buildtype=release  build_release
	meson compile -j 8 -C build_release

release-mingw:
	meson setup --cross-file x86_64-w64-mingw32.txt --buildtype=release --wipe build-mingw_release
	meson compile -j 8 -C build-mingw_release

setup:
	meson setup -Db_sanitize=address,undefined --wipe build 

dev:
	meson compile -j 8 -C build

test: 
	meson test neosql-core: -C build

dev-mingw:
	meson setup --cross-file x86_64-w64-mingw32.txt --wipe build-mingw
	meson compile -j 8 -C build-mingw


test-mingw: dev-mingw 
	meson test neosql-core: -C build-mingw

install: release release-mingw
	meson install -C build_release --destdir .
	meson install -C build-mingw_release --destdir .
