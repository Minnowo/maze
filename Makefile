.PHONY: all build build_debug

SOURCE_DIR       := $(abspath .)
BUILD_DIR        := build
CMAKE_BUILD_TYPE := Release

NAME       := main
TARGET     := ${BUILD_DIR}/$(NAME)

all: build


build:
	CC="$(CC)" CXX="$(CXX)" cmake -B $(BUILD_DIR) \
	   -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) \
	   -DDESKTOP_TARGET=$(DESKTOP_TARGET)
	CC="$(CC)" CXX="$(CXX)" make -C  $(BUILD_DIR) -j$(shell nproc)
	cp $(BUILD_DIR)/compile_commands.json $(SOURCE_DIR) || true
