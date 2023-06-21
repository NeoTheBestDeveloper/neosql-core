.PHONY: clean, test 

clean:
	rm -rf build

test:
	meson test neosql-core: -C build
