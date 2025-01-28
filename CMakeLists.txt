cmake_minimum_required(VERSION 3.10)

project(h264dump VERSION 0.1)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_C_FLAGS "-Wnullability-completeness")

set(BIN "h264dump")
set(LIB "libh264dump")

set(SRC_DIR
    "${PROJECT_SOURCE_DIR}/h264dump/src"
    "${PROJECT_BINARY_DIR}/h264dump/src")

set(TEST_DIR "${PROJECT_SOURCE_DIR}/h264dump/tests")

enable_testing()
add_subdirectory(h264dump)
