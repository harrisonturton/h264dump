cmake_minimum_required(VERSION 3.10)

project(h264dump VERSION 0.1)

# Require c23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)

# Generate version.hpp from the version in CMakeLists.txt
configure_file(include/version.h.in include/version.h @ONLY)

# Produce the h264dump binary
add_executable(h264dump src/main.c src/buf.c)
target_include_directories(h264dump PUBLIC "${PROJECT_BINARY_DIR}/include")
target_include_directories(h264dump PUBLIC "${PROJECT_BINARY_DIR}/src")