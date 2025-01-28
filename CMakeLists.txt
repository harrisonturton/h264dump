cmake_minimum_required(VERSION 3.10)

project(h264dump VERSION 0.1)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_C_FLAGS "-Wnullability-completeness")

set(BIN "h264dump")
set(LIB "libh264dump")

set(INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

configure_file(
    include/version.h.in
    include/version.h @ONLY
)

add_subdirectory(src)

function(create_test NAME SRCS)
    add_executable(${NAME} "${SRCS}")
    target_include_directories(${NAME}
        PUBLIC "${INCLUDE_DIR}"
        PRIVATE "${SRC_DIR}")
    target_link_libraries(${NAME}
        PRIVATE ${LIB})

    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

enable_testing()
create_test(buffer test/buf/buffer_test.c)
