CMAKE_BUILD_TYPE ?= Debug
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
CMAKE_EXTRA_FLAGS ?=

GEN_CMD := cmake -S. -Bbuild -GNinja ${CMAKE_FLAGS} ${CMAKE_EXTRA_FLAGS}
BUILD_TARGET_CMD := cmake --build build --target

all: spleeter

build/.ran-cmake:
	mkdir -p build
	${GEN_CMD}
	touch $@

spleeter: build/.ran-cmake
	${BUILD_TARGET_CMD} all

test: spleeter
	${BUILD_TARGET_CMD} test

clean: | build/.ran-cmake
	${BUILD_TARGET_CMD} clean

distclean:
	rm -rf build
