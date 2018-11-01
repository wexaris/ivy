# This Makefile is just a shorthand for
# running cmake in debug or release mode

# Run everything in one command shell
# Otherwise cd won't work
.ONESHELL:

all: debug

debug:
	mkdir -p build/debug && cd build/debug
	cmake -DCMAKE_BUILD_TYPE=Debug ../../
	echo && make

release: clean
	mkdir -p build/release && cd build/release
	cmake -DCMAKE_BUILD_TYPE=Release ../../
	echo && make

clean:
	rm -rf build/debug bin/debug
	rm -rf build/release bin/release