.PHONY: clean, test 

clean:
	rm -rf build
	rm -rf build_release

release:
	meson setup --reconfigure --buildtype=release  build_release
	meson compile -j 8 -C build_release

setup:
	meson setup --reconfigure build

dev:
	meson compile -j 8 -C build

test: dev
	meson test neosql-core: -C build

install: release 
	meson install -C build_release 
