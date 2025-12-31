all: build_all
build_all:
	@echo Building...
	cmake . -B build -DBUILD_SHARED_LIBS=ON
	cmake --build build -j 8
test: build_all
	LD_LIBRARY_PATH=./build:$LD_LIBRARY_PATH ctest --test-dir build/tests --output-on-failure
